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

#include "asynctask_queue.h"
#include <assert.h>
#include <unistd.h>
#include "abstract_asynctask_thread_pool.h"
#include "lck.h"
#include "log.h"
#include "sthread.h"

supla_asynctask_queue::supla_asynctask_queue(void) {
  lck = lck_init();
  eh = eh_init();
  thread = sthread_simple_run(loop, this, 0);
}

supla_asynctask_queue::~supla_asynctask_queue(void) {
  sthread_twf(thread);
  release_pools();
  release_tasks();

  lck_free(lck);
  eh_free(eh);
}

// static
void supla_asynctask_queue::loop(void *_queue, void *q_sthread) {
  supla_asynctask_queue *queue = static_cast<supla_asynctask_queue *>(_queue);
  if (!queue) {
    return;
  }

  while (sthread_isterminated(q_sthread) == 0) {
    queue->iterate();
  }
}

void supla_asynctask_queue::release_pools(void) {
  do {
    lck_lock(lck);
    if (pools.size()) {
      delete pools.front();
    }
    lck_unlock(lck);
  } while (pool_count());
}

void supla_asynctask_queue::release_tasks(void) {
  do {
    lck_lock(lck);
    if (tasks.size()) {
      delete tasks.front();
    }
    lck_unlock(lck);
  } while (total_count());
}

bool supla_asynctask_queue::task_exists(supla_abstract_asynctask *task) {
  bool result = false;
  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if (*it == task) {
      result = true;
      break;
    }
  }

  lck_unlock(lck);

  return result;
}

void supla_asynctask_queue::add_task(supla_abstract_asynctask *task) {
  assert(task->get_pool() != NULL);
  assert(pool_exists(task->get_pool()));

  lck_lock(lck);
  task->task_will_added();

  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if ((*it)->get_priority() < task->get_priority()) {
      tasks.insert(it, task);
      task = NULL;
      break;
    }
  }

  if (task) {
    tasks.push_back(task);
  }
  lck_unlock(lck);

  eh_raise_event(eh);
}

void supla_asynctask_queue::remove_task(supla_abstract_asynctask *task) {
  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if (*it == task) {
      tasks.erase(it);
      break;
    }
  }
  lck_unlock(lck);
}

bool supla_asynctask_queue::pool_exists(
    supla_abstract_asynctask_thread_pool *pool) {
  bool result = false;
  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask_thread_pool *>::iterator it =
           pools.begin();
       it != pools.end(); ++it) {
    if (*it == pool) {
      result = true;
      break;
    }
  }

  lck_unlock(lck);

  return result;
}

void supla_asynctask_queue::register_pool(
    supla_abstract_asynctask_thread_pool *pool) {
  lck_lock(lck);
  if (!pool_exists(pool)) {
    pools.push_back(pool);
  }

  lck_unlock(lck);
}

void supla_asynctask_queue::unregister_pool(
    supla_abstract_asynctask_thread_pool *pool) {
  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask_thread_pool *>::iterator it =
           pools.begin();
       it != pools.end(); ++it) {
    if (*it == pool) {
      pools.erase(it);
      break;
    }
  }
  lck_unlock(lck);
}

supla_abstract_asynctask *supla_asynctask_queue::pick(
    supla_abstract_asynctask_thread_pool *pool) {
  supla_abstract_asynctask *result = NULL;

  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if ((*it)->get_state() == STA_STATE_WAITING && (*it)->get_pool() == pool &&
        (*it)->time_left_usec(NULL) <= 0) {
      result = *it;
      result->pick();
      break;
    }
  }

  lck_unlock(lck);
  return result;
}

void supla_asynctask_queue::iterate(void) {
  struct timeval now;
  gettimeofday(&now, NULL);

  long long wait_time = 1000000;

  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    long long time_left = (*it)->time_left_usec(&now);
    if (time_left <= 0) {
      if ((*it)->get_state() == STA_STATE_WAITING) {
        (*it)->get_pool()->execution_request(*it);
      }
    } else if (time_left < wait_time) {
      wait_time = time_left;
    }
  }
  lck_unlock(lck);

  if (wait_time < 10000) {
    wait_time = 10000;
  }

  eh_wait(eh, wait_time);
}

unsigned int supla_asynctask_queue::pool_count(void) {
  lck_lock(lck);
  unsigned int result = pools.size();
  lck_unlock(lck);

  return result;
}

unsigned int supla_asynctask_queue::total_count(void) {
  lck_lock(lck);
  unsigned int result = tasks.size();
  lck_unlock(lck);

  return result;
}

unsigned int supla_asynctask_queue::waiting_count(void) {
  unsigned int result = 0;
  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if ((*it)->get_state() == STA_STATE_WAITING) {
      result++;
    }
  }
  lck_unlock(lck);

  return result;
}

unsigned int supla_asynctask_queue::thread_count(void) {
  unsigned int result = 0;
  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask_thread_pool *>::iterator it =
           pools.begin();
       it != pools.end(); ++it) {
    result += (*it)->thread_count();
  }
  lck_unlock(lck);

  return result;
}

void supla_asynctask_queue::raise_event(void) { eh_raise_event(eh); }

supla_abstract_asynctask *supla_asynctask_queue::find_task(
    supla_abstract_asynctask_search_condition *cnd) {
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if (cnd->condition_met(*it)) {
      return *it;
    }
  }
  return NULL;
}

bool supla_asynctask_queue::get_task_state(
    async_task_state *state, supla_abstract_asynctask_search_condition *cnd) {
  if (!state) {
    return false;
  }

  lck_lock(lck);
  supla_abstract_asynctask *task = find_task(cnd);
  if (task != NULL) {
    *state = task->get_state();
  }
  lck_unlock(lck);

  return task != NULL;
}

unsigned int supla_asynctask_queue::get_task_count(
    supla_abstract_asynctask_search_condition *cnd) {
  unsigned int result = 0;
  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if (cnd->condition_met(*it)) {
      result++;
    }
  }
  lck_unlock(lck);
  return result;
}

void supla_asynctask_queue::cancel_task(
    supla_abstract_asynctask_search_condition *cnd) {
  lck_lock(lck);
  supla_abstract_asynctask *task = find_task(cnd);
  if (task != NULL) {
    bool release = task->release_immediately_after_execution() &&
                   (task->get_state() == STA_STATE_INIT ||
                    task->get_state() == STA_STATE_WAITING);

    task->cancel();

    if (release) {
      delete task;
    }
  }
  lck_unlock(lck);
}
