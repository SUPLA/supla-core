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
#include "serverstatus.h"
#include "sthread.h"

using std::function;
using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_asynctask_queue::supla_asynctask_queue(void) {
  this->lck = lck_init();
  this->observer_lck = lck_init();
  this->eh = eh_init();
  this->last_iterate_time_sec = 0;
  this->thread = sthread_simple_run(loop, this, 0);
}

supla_asynctask_queue::~supla_asynctask_queue(void) {
  sthread_twf(thread);
  release_pools();

  lck_lock(lck);
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    (*it)->cancel();
  }
  tasks.clear();
  lck_unlock(lck);

  lck_free(lck);
  lck_free(observer_lck);
  eh_free(eh);
}

// static
supla_asynctask_queue *supla_asynctask_queue::_global_instance = NULL;

// static
supla_asynctask_queue *supla_asynctask_queue::global_instance(void) {
  if (_global_instance == NULL) {
    _global_instance = new supla_asynctask_queue();
  }

  return _global_instance;
}

// static
void supla_asynctask_queue::global_instance_release(void) {
  if (_global_instance) {
    delete _global_instance;
    _global_instance = NULL;
  }
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

bool supla_asynctask_queue::task_exists(supla_abstract_asynctask *task) {
  bool result = false;
  lck_lock(lck);
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if (it->get() == task) {
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

  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if ((*it)->get_priority() < task->get_priority()) {
      tasks.insert(it, shared_ptr<supla_abstract_asynctask>(task));
      task = NULL;
      break;
    }
  }

  if (task) {
    tasks.push_back(shared_ptr<supla_abstract_asynctask>(task));
  }
  lck_unlock(lck);

  eh_raise_event(eh);
}

void supla_asynctask_queue::remove_task(supla_abstract_asynctask *task) {
  lck_lock(lck);
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if (it->get() == task) {
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
  for (vector<supla_abstract_asynctask_thread_pool *>::iterator it =
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
  for (vector<supla_abstract_asynctask_thread_pool *>::iterator it =
           pools.begin();
       it != pools.end(); ++it) {
    if (*it == pool) {
      pools.erase(it);
      break;
    }
  }
  lck_unlock(lck);
}

shared_ptr<supla_abstract_asynctask> supla_asynctask_queue::pick(
    supla_abstract_asynctask_thread_pool *pool) {
  shared_ptr<supla_abstract_asynctask> result;

  lck_lock(lck);
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if ((*it)->get_state() == supla_asynctask_state::WAITING &&
        (*it)->get_pool() == pool && (*it)->time_left_usec(NULL) <= 0) {
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

  lck_lock(lck);
  last_iterate_time_sec = now.tv_sec;
  lck_unlock(lck);

  long long wait_time = 1000000;

  lck_lock(lck);
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    long long time_left = (*it)->time_left_usec(&now);
    if (time_left <= 0) {
      if ((*it)->get_state() == supla_asynctask_state::WAITING) {
        (*it)->get_pool()->execution_request(it->get());
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
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if ((*it)->get_state() == supla_asynctask_state::WAITING) {
      result++;
    }
  }
  lck_unlock(lck);

  return result;
}

unsigned int supla_asynctask_queue::thread_count(void) {
  unsigned int result = 0;
  lck_lock(lck);
  for (auto it = pools.begin(); it != pools.end(); ++it) {
    result += (*it)->thread_count();
  }
  lck_unlock(lck);

  return result;
}

void supla_asynctask_queue::raise_event(void) { eh_raise_event(eh); }

supla_abstract_asynctask *supla_asynctask_queue::find_task(
    supla_abstract_asynctask_search_condition *cnd) {
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if (cnd->condition_met(it->get())) {
      return it->get();
    }
  }
  return NULL;
}

bool supla_asynctask_queue::get_task_state(
    supla_asynctask_state *state,
    supla_abstract_asynctask_search_condition *cnd) {
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
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if (cnd->condition_met(it->get())) {
      result++;
    }
  }
  lck_unlock(lck);
  return result;
}

bool supla_asynctask_queue::task_exists(
    supla_abstract_asynctask_search_condition *cnd) {
  lck_lock(lck);
  bool result = find_task(cnd) != NULL;
  lck_unlock(lck);

  return result;
}

void supla_asynctask_queue::cancel_tasks(
    supla_abstract_asynctask_search_condition *cnd) {
  lck_lock(lck);

  vector<shared_ptr<supla_abstract_asynctask> > found;

  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if (cnd->condition_met(it->get())) {
      found.push_back(*it);
    }
  }

  for (auto it = found.begin(); it != found.end(); ++it) {
    (*it)->cancel();

    if ((*it)->release_immediately_after_execution()) {
      remove_task(it->get());
    }
  }
  lck_unlock(lck);
}

void supla_asynctask_queue::add_observer(
    supla_abstract_asynctask_observer *observer) {
  lck_lock(observer_lck);
  bool exists = false;
  for (auto it = observers.begin(); it != observers.end(); ++it) {
    if (*it == observer) {
      exists = true;
      break;
    }
  }

  if (!exists) {
    observers.push_back(observer);
  }

  lck_unlock(observer_lck);
}

void supla_asynctask_queue::remove_observer(
    supla_abstract_asynctask_observer *observer) {
  lck_lock(observer_lck);
  for (auto it = observers.begin(); it != observers.end(); ++it) {
    if (*it == observer) {
      observers.erase(it);
      break;
    }
  }
  lck_unlock(observer_lck);
}

void supla_asynctask_queue::on_task_started(supla_abstract_asynctask *task) {
  lck_lock(observer_lck);
  for (auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)->on_asynctask_started(task);
  }
  lck_unlock(observer_lck);
}

void supla_asynctask_queue::on_task_finished(supla_abstract_asynctask *task) {
  lck_lock(observer_lck);
  for (auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)->on_asynctask_finished(task);
  }
  lck_unlock(observer_lck);
}

bool supla_asynctask_queue::access_task(
    supla_abstract_asynctask_search_condition *cnd,
    function<void(supla_abstract_asynctask *)> on_task) {
  bool result = false;
  lck_lock(lck);
  supla_abstract_asynctask *task = find_task(cnd);
  if (task != NULL) {
    result = true;
    on_task(task);
  }
  lck_unlock(lck);
  return result;
}

weak_ptr<supla_abstract_asynctask> supla_asynctask_queue::get_weak_ptr(
    supla_abstract_asynctask *task) {
  weak_ptr<supla_abstract_asynctask> result;

  lck_lock(lck);
  for (auto it = tasks.begin(); it != tasks.end(); ++it) {
    if (it->get() == task) {
      result = *it;
      break;
    }
  }
  lck_unlock(lck);

  return result;
}

void supla_asynctask_queue::log_stuck_warning(void) {
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
  struct timeval now;
  gettimeofday(&now, NULL);

  lck_lock(lck);
  int time = now.tv_sec - last_iterate_time_sec;
  lck_unlock(lck);

  if (time > 10) {
    supla_log(LOG_WARNING, "AsyncTask Queue iteration is stuck!");
  }
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
}
