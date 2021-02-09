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
#include "abstract_asynctask_thread_pool.h"
#include "lck.h"

supla_asynctask_queue::supla_asynctask_queue(void) {
  lck = lck_init();
  eh = eh_init();
}

supla_asynctask_queue::~supla_asynctask_queue(void) {
  lck_free(lck);
  eh_free(eh);
}

void supla_asynctask_queue::sort_tasks(void) {
  lck_lock(lck);
  lck_unlock(lck);
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
  tasks.push_back(task);
  lck_unlock(lck);

  sort_tasks();
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
    supla_abstract_asynctask_thread_pool *pool) {}

supla_abstract_asynctask *supla_asynctask_queue::pick(
    supla_abstract_asynctask_thread_pool *pool) {
  supla_abstract_asynctask *result = NULL;

  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    if ((*it)->get_state() == STA_STATE_WAITING && (*it)->get_pool() == pool &&
        (*it)->time_left_usec(NULL) <= 0) {
      result = *it;
      break;
    }
  }

  lck_unlock(lck);
  return result;
}

void supla_asynctask_queue::iterate(void *q_sthread) {
  struct timeval now;
  gettimeofday(&now, NULL);

  long long wait_time = 1000000;

  lck_lock(lck);
  for (std::vector<supla_abstract_asynctask *>::iterator it = tasks.begin();
       it != tasks.end(); ++it) {
    long long time_left = (*it)->time_left_usec(&now);
    if (time_left <= 0) {
      if ((*it)->get_state() == STA_STATE_WAITING) {
        (*it)->get_pool()->execution_request();
      }
    } else if (time_left < wait_time) {
      wait_time = time_left;
    }
  }
  lck_unlock(lck);

  if (wait_time > 10000) {
    wait_time = 10000;
  }

  eh_wait(eh, wait_time);
}
