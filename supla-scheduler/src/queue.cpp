/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "ipcclient.h"
#include "lck.h"
#include "log.h"
#include "queue.h"
#include "safearray.h"
#include "schedulercfg.h"
#include "sthread.h"
#include "worker.h"

char queue_loop_s_exec_arr_free(void *s_exec) {
  if (((s_exec_t *)s_exec)->action_param)
    free(((s_exec_t *)s_exec)->action_param);

  free(s_exec);
  return 1;
}

char queue_loop_worker_thread_twt(void *worker_sthread) {
  sthread_twf(worker_sthread);
  return 1;
}

char queue_loop_worker_thread_cnd(void *worker_sthread) {
  if (sthread_isfinished(worker_sthread) == 1) {
    sthread_free(worker_sthread);
    return 1;
  }

  return 0;
}

void queue_loop_worker_execute(void *worker, void *sthread) {
  database::thread_init();
  ((s_worker *)worker)->execute(sthread);
}

void queue_loop_worker_finish(void *worker, void *sthread) {
  delete (s_worker *)worker;
  database::thread_end();
}

queue::queue(void *user, void *q_sthread) {
  this->user = user;
  this->q_sthread = q_sthread;

  timer.tv_sec = 0;
  timer.tv_usec = 0;

  timer_one_min.tv_sec = 0;
  timer_one_min.tv_usec = 0;

  loop_eh = eh_init();

  max_workers = scfg_int(CFG_MAX_WORKERS);
  max_job_per_second = scfg_int(CFG_MAX_JOB_PER_SECOND);

  job_counter = 0;
  total_fetch_count = 0;
  wait_for_fetch_counter = 0;

  lck = lck_init();
  workers_thread_arr = safe_array_init();
  s_exec_arr = safe_array_init();
  db = new database();
}

queue::~queue() {
  safe_array_clean(workers_thread_arr, queue_loop_worker_thread_twt);
  safe_array_free(workers_thread_arr);

  safe_array_clean(s_exec_arr, queue_loop_s_exec_arr_free);
  safe_array_free(s_exec_arr);

  delete db;

  lck_free(lck);
  eh_free(loop_eh);
}

void queue::set_overdue_result(void) {
  database *_db = new database();
  if (_db->connect()) {
    _db->set_expired_result(EXPIRE_TIME);
  }
  delete _db;
}

void queue::set_zombie_result(void) {
  database *_db = new database();
  if (_db->connect()) {
    _db->set_zombie_result(ZOMBIE_TIME);
  }
  delete _db;
}

void queue::new_worker(void) {
  if (sthread_isterminated(q_sthread) ||
      safe_array_count(workers_thread_arr) >= max_workers)
    return;

  Tsthread_params stp;

  stp.execute = queue_loop_worker_execute;
  stp.finish = queue_loop_worker_finish;
  stp.user_data = new s_worker(this);
  stp.free_on_finish = 0;
  stp.initialize = NULL;

  safe_array_add(workers_thread_arr, sthread_run(&stp));
}

void queue::raise_loop_event(void) { eh_raise_event(loop_eh); }

bool queue::limit_exceeded(void) {
  bool limit_exceeded = false;

  lck_lock(lck);

  if (job_counter >= max_job_per_second) {
    limit_exceeded = true;
    // supla_log(LOG_DEBUG, "limit exceeded");
  }

  lck_unlock(lck);

  return limit_exceeded;
}

s_exec_t queue::get_job(void) {
  s_exec_t result;
  memset(&result, 0, sizeof(s_exec_t));

  if (sthread_isterminated(q_sthread) || limit_exceeded()) return result;

  s_exec_t *s_exec = (s_exec_t *)safe_array_pop(s_exec_arr);

  if (s_exec != NULL) {
    lck_lock(lck);
    wait_for_fetch_counter++;
    job_counter++;

    gettimeofday(&last_get, NULL);
    lck_unlock(lck);

    result = *s_exec;

    if (result.action_param != NULL) {
      result.action_param = strdup(s_exec->action_param);
      free(s_exec->action_param);
    }

    free(s_exec);
  }

  if (safe_array_count(s_exec_arr) > 0) new_worker();

  return result;
}

bool queue::wait_for_fetch(void) {
  bool result = false;

  lck_lock(lck);

  if (safe_array_count(workers_thread_arr) == 0) wait_for_fetch_counter = 0;

  if (wait_for_fetch_counter > 0) {
    struct timeval now;
    gettimeofday(&now, NULL);

    if (last_get.tv_sec - now.tv_sec < 5) {
      result = true;
    }
  }

  lck_unlock(lck);

  return result;
}

void queue::mark_fetched() {
  lck_lock(lck);
  total_fetch_count++;
  wait_for_fetch_counter--;

  if (wait_for_fetch_counter < 0) wait_for_fetch_counter = 0;

  lck_unlock(lck);
}

void queue::load(void) {
  if (safe_array_count(s_exec_arr) > 0 || wait_for_fetch() || limit_exceeded())
    return;

  int limit = max_workers * 10 - safe_array_count(s_exec_arr);

  if (limit <= 0) return;

  if (!db->connect()) return;

  struct timeval now;
  gettimeofday(&now, NULL);

  if (now.tv_sec - timer_one_min.tv_sec >= 60) {
    timer_one_min = now;

    set_zombie_result();
    set_overdue_result();
  }

  db->get_s_executions(s_exec_arr, limit);
  db->disconnect();

  // supla_log(LOG_DEBUG, "LOAD");
}

void queue::loop(void) {
  int a = 0;
  struct timeval now;
  now.tv_sec = 0;
  now.tv_usec = 0;

  while (sthread_isterminated(q_sthread) == 0) {
    gettimeofday(&now, NULL);

    if (now.tv_sec - timer.tv_sec >= 1) {
      lck_lock(lck);
      job_counter = 0;
      lck_unlock(lck);

      timer = now;
    }

    safe_array_clean(workers_thread_arr, queue_loop_worker_thread_cnd);
    load();

    if (safe_array_count(s_exec_arr) > 0 &&
        safe_array_count(workers_thread_arr) == 0) {
      new_worker();
    }

    eh_wait(loop_eh, 1000000);
  }

  safe_array_lock(workers_thread_arr);
  for (a = 0; a < safe_array_count(workers_thread_arr); a++) {
    sthread_terminate(safe_array_get(workers_thread_arr, a));
  }
  safe_array_unlock(workers_thread_arr);

  while (safe_array_count(workers_thread_arr)) {
    safe_array_clean(workers_thread_arr, queue_loop_worker_thread_cnd);
    eh_wait(loop_eh, 1000);
  }
}

void queue::print_statistics(void) {
  lck_lock(lck);
  supla_log(LOG_DEBUG, "Statistics");
  supla_log(LOG_DEBUG, "Total fetch count: %i", total_fetch_count);
  lck_unlock(lck);
}

void queue_loop(void *user, void *q_sthread) {
  database::thread_init();

  queue *q = new queue(user, q_sthread);
  q->loop();
  q->print_statistics();
  delete q;

  database::thread_end();
}
