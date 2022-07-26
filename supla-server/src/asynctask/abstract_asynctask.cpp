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

#include "abstract_asynctask.h"

#include <assert.h>

#include "abstract_asynctask_thread_pool.h"
#include "asynctask_queue.h"
#include "lck.h"

supla_abstract_asynctask::supla_abstract_asynctask(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    short priority, bool release_immediately) {
  init(queue, pool, priority, release_immediately);
  queue->add_task(this);
}

supla_abstract_asynctask::supla_abstract_asynctask(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool) {
  init(queue, pool, 0, true);
  queue->add_task(this);
}

void supla_abstract_asynctask::init(supla_asynctask_queue *queue,
                                    supla_abstract_asynctask_thread_pool *pool,
                                    short priority, bool release_immediately) {
  assert(queue);
  assert(pool);

  this->lck = lck_init();
  this->observable = false;
  this->observer_notified = false;
  this->delay_usec = 0;
  this->started_at = {};
  this->execution_start_time = {};
  this->timeout_usec = 0;
  this->queue = queue;
  this->pool = pool;
  this->priority = priority;
  this->release_immediately = release_immediately;

  gettimeofday(&started_at, NULL);
}

supla_abstract_asynctask::~supla_abstract_asynctask(void) {
  on_task_finished();
  lck_free(lck);
}

void supla_abstract_asynctask::lock(void) { lck_lock(lck); }

void supla_abstract_asynctask::unlock(void) { lck_unlock(lck); }

void supla_abstract_asynctask::set_observable(void) {
  lock();
  bool prev = observable;
  if (!observable) {
    observable = true;
  }
  unlock();

  if (!prev) {
    queue->on_task_started(this);
  }
}

bool supla_abstract_asynctask::is_observable(void) {
  lock();
  bool result = observable;
  unlock();

  return result;
}

supla_asynctask_queue *supla_abstract_asynctask::get_queue(void) {
  return queue;
}

supla_abstract_asynctask_thread_pool *supla_abstract_asynctask::get_pool(void) {
  return pool;
}

short supla_abstract_asynctask::get_priority(void) { return priority; }

supla_asynctask_state supla_abstract_asynctask::get_state(void) {
  lock();
  supla_asynctask_state result = state;
  unlock();

  return result;
}

struct timeval supla_abstract_asynctask::get_started_at(void) {
  lock();
  struct timeval result = started_at;
  unlock();
  return result;
}

long long supla_abstract_asynctask::get_delay_usec(void) {
  lock();
  long long result = delay_usec;
  unlock();
  return result;
}

void supla_abstract_asynctask::set_delay_usec(long long delay_usec) {
  lock();
  gettimeofday(&execution_start_time, NULL);
  this->delay_usec = delay_usec;

  if (delay_usec > 0) {
    execution_start_time.tv_sec += delay_usec / 1000000;
    execution_start_time.tv_usec += delay_usec % 1000000;
  }

  unlock();
  if (get_state() != supla_asynctask_state::EXECUTING) {
    queue->raise_event();
  }
}

void supla_abstract_asynctask::set_timeout(unsigned long long timeout_usec) {
  lock();
  this->timeout_usec = timeout_usec;
  unlock();
}

unsigned long long supla_abstract_asynctask::get_timeout(void) {
  lock();
  long long result = timeout_usec;
  unlock();
  return result;
}

long long supla_abstract_asynctask::time_left_usec(struct timeval *now) {
  struct timeval _now;
  if (!now) {
    gettimeofday(&_now, NULL);
    now = &_now;
  }

  lock();
  long long result = 0;
  if (execution_start_time.tv_sec || execution_start_time.tv_usec) {
    result = (execution_start_time.tv_sec * (long long)1000000 +
              execution_start_time.tv_usec) -
             (now->tv_sec * (long long)1000000 + now->tv_usec);
  }
  unlock();

  return result;
}

void supla_abstract_asynctask::set_waiting(void) {
  lock();
  if (state == supla_asynctask_state::INIT) {
    state = supla_asynctask_state::WAITING;
  }
  unlock();
  queue->raise_event();
}

bool supla_abstract_asynctask::pick(void) {
  lock();
  bool result = state == supla_asynctask_state::WAITING;
  if (result) {
    state = supla_asynctask_state::PICKED;
  }
  unlock();

  return result;
}

void supla_abstract_asynctask::execute(void) {
  lock();
  bool exec_allowed = state == supla_asynctask_state::PICKED;

  if (exec_allowed && timeout_usec) {
    long long time_left = time_left_usec(NULL);
    if (time_left < 0) {
      time_left *= -1;
      if ((long long unsigned)time_left >= timeout_usec) {
        state = supla_asynctask_state::TIMEOUT;
        exec_allowed = false;
      }
    }
  }

  if (exec_allowed) {
    state = supla_asynctask_state::EXECUTING;
  }
  unlock();

  if (!exec_allowed) {
    return;
  }

  bool exec_again = false;
  bool result = _execute(&exec_again);

  lock();
  if (state != supla_asynctask_state::CANCELED) {
    if (exec_again) {
      state = supla_asynctask_state::WAITING;
      set_delay_usec(delay_usec);
    } else if (result) {
      state = supla_asynctask_state::SUCCESS;
    } else {
      state = supla_asynctask_state::FAILURE;
    }
  }
  unlock();

  if (is_finished()) {
    on_task_finished();
  }
}

void supla_abstract_asynctask::on_task_finished(void) {
  if (is_observable() && !observer_notified) {
    observer_notified = true;
    queue->on_task_finished(this);
  }
}

void supla_abstract_asynctask::cancel(void) {
  lock();
  state = supla_asynctask_state::CANCELED;
  unlock();
}

bool supla_abstract_asynctask::is_finished(void) {
  lock();
  bool result = false;

  switch (state.get_state()) {
    case supla_asynctask_state::SUCCESS:
    case supla_asynctask_state::FAILURE:
    case supla_asynctask_state::TIMEOUT:
    case supla_asynctask_state::CANCELED:
      result = true;
      break;
    default:
      break;
  }

  unlock();

  return result;
}

bool supla_abstract_asynctask::release_immediately_after_execution(void) {
  lock();
  bool result = release_immediately;
  unlock();
  return result;
}
