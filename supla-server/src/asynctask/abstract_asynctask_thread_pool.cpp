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

#include "abstract_asynctask_thread_pool.h"

#include <assert.h>
#include <unistd.h>

#include <memory>

#include "abstract_asynctask.h"
#include "asynctask_queue.h"
#include "lck.h"
#include "log.h"
#include "metrics.h"
#include "sthread.h"

#define WARNING_MIN_FREQ_SEC 5
#define PICK_RETRY_LIMIT 3
#define PICK_RETRY_DELAY_USEC 10000
#define PICK_WARNING_USEC 100000
#define REMOVE_WARNING_USEC 100000

using std::shared_ptr;
using std::vector;

supla_abstract_asynctask_thread_pool::supla_abstract_asynctask_thread_pool(
    supla_asynctask_queue *queue) {
  assert(queue);
  this->lck = lck_init();
  this->queue = queue;
  this->terminated = false;
  this->_overload_count = 0;
  this->_exec_count = 0;
  this->_highest_number_of_threads = 0;
  this->warinig_time.tv_sec = 0;
  this->warinig_time.tv_usec = 0;
  this->holded = false;

  queue->register_pool(this);
}

supla_abstract_asynctask_thread_pool::~supla_abstract_asynctask_thread_pool(
    void) {
  terminate();

  int n = 0;
  while (thread_count()) {
    usleep(10000);
    n++;
    if (n == 100) {
      terminate();
    }
    if (n == 500) {
      supla_log(LOG_ERR,
                "Elapsed time waiting for the threads in the pool to stop. "
                "Threads left: %i",
                thread_count());
    }
  }

  queue->unregister_pool(this);
  lck_free(lck);
}

int supla_abstract_asynctask_thread_pool::tasks_per_thread(void) { return 0; }

bool supla_abstract_asynctask_thread_pool::should_keep_alive(
    unsigned long long usec_since_last_exec, size_t thread_count) {
  return false;
}

void supla_abstract_asynctask_thread_pool::execution_request(
    supla_abstract_asynctask *task) {
  if (is_terminated() || is_holded()) {
    return;
  }

  size_t req_count = 0;

  {
    bool already_exists = false;

    lck_lock(lck);

    if (requests.size() && !threads.size()) {
      supla_log(LOG_WARNING,
                "There are %i task(s) in the %s pool but there are no threads.",
                pool_name().c_str(), requests.size());
    }

    for (auto it = requests.begin(); it != requests.end(); ++it) {
      if (*it == task) {
        already_exists = true;
        break;
      }
    }

    if (!already_exists) {
      requests.push_back(task);
    }

    req_count = requests.size();

    lck_unlock(lck);

    if (already_exists) {
      return;
    }
  }

  bool overload_warning = false;

  lck_lock(lck);
  if (threads.size() < thread_count_limit()) {
    if (!tasks_per_thread() ||
        req_count > tasks_per_thread() * threads.size()) {
      Tsthread_params p;
      p.user_data = this;
      p.free_on_finish = 1;
      p.execute = _execute;
      p.finish = _on_thread_finish;
      p.initialize = NULL;

      void *thread = nullptr;
      sthread_run(&p, &thread);
      if (thread) {
        threads.push_back(thread);
        if (threads.size() > _highest_number_of_threads) {
          _highest_number_of_threads = threads.size();
        }
      }
    }

  } else {
    _overload_count++;
    struct timeval now;
    gettimeofday(&now, NULL);

    if (now.tv_sec - warinig_time.tv_sec >= WARNING_MIN_FREQ_SEC) {
      warinig_time = now;
      overload_warning = true;
    }
  }
  lck_unlock(lck);

  if (overload_warning) {
    supla_log(LOG_WARNING,
              "The thread pool for asynchronous tasks is overloaded. Pool "
              "Name: %s Limit: %li Counter: %li",
              pool_name().c_str(), thread_count_limit(), overload_count());
  }
}

void supla_abstract_asynctask_thread_pool::remove_task(
    supla_abstract_asynctask *task) {
  lck_lock(lck);
  for (auto it = requests.begin(); it != requests.end(); ++it) {
    if (*it == task) {
      requests.erase(it);
      break;
    }
  }
  lck_unlock(lck);
}

// static
void supla_abstract_asynctask_thread_pool::_execute(void *_pool,
                                                    void *sthread) {
  static_cast<supla_abstract_asynctask_thread_pool *>(_pool)->execute(sthread);
}

void supla_abstract_asynctask_thread_pool::execute(void *sthread) {
  bool iterate = true;

  supla_asynctask_thread_bucket *bucket = get_bucket();
  struct timeval last_exec_time = {};

  do {
    shared_ptr<supla_abstract_asynctask> task;

    unsigned long long time_usec = supla_metrics::measure_the_time_in_usec(
        [&task, this]() -> void { task = queue->pick(this); });

    if (time_usec >= PICK_WARNING_USEC) {
      supla_log(LOG_WARNING,
                "The time to pick the task in the %s pool was %llu usec.",
                pool_name().c_str(), time_usec);
    }

    if (task) {
      task->execute(bucket);
      lck_lock(lck);
      _exec_count++;
      lck_unlock(lck);

      if (task->is_finished()) {
        queue->remove_task(task);
      }

      time_usec = supla_metrics::measure_the_time_in_usec(
          [&task, this]() -> void { remove_task(task.get()); });

      if (time_usec >= REMOVE_WARNING_USEC) {
        supla_log(LOG_WARNING,
                  "The time to remove the task in the %s pool was %llu usec.",
                  pool_name().c_str(), time_usec);
      }

      gettimeofday(&last_exec_time, nullptr);
    }

    if (task == NULL) {
      if (!sthread_isterminated(sthread)) {
        usleep(PICK_RETRY_DELAY_USEC);
      }
    }

    lck_lock(lck);
    iterate = false;
    if (!sthread_isterminated(sthread)) {
      if (threads.size() <= requests.size()) {
        iterate = true;
      } else {
        struct timeval now = {};
        gettimeofday(&now, nullptr);

        iterate = should_keep_alive(
            (now.tv_sec * 1000000UL + now.tv_usec) -
                (last_exec_time.tv_sec * 1000000UL + last_exec_time.tv_usec),
            threads.size());
      }
    }
    lck_unlock(lck);
  } while (iterate);

  if (bucket) {
    delete bucket;
  }
}

// static
void supla_abstract_asynctask_thread_pool::_on_thread_finish(void *_pool,
                                                             void *sthread) {
  static_cast<supla_abstract_asynctask_thread_pool *>(_pool)->on_thread_finish(
      sthread);
}

void supla_abstract_asynctask_thread_pool::on_thread_finish(void *sthread) {
  lck_lock(lck);
  for (auto it = threads.begin(); it != threads.end(); ++it) {
    if (*it == sthread) {
      threads.erase(it);
      break;
    }
  }

  if (threads.size() == 0) {
    requests.clear();
  }

  lck_unlock(lck);
}

unsigned int supla_abstract_asynctask_thread_pool::thread_count(void) {
  lck_lock(lck);
  unsigned int result = threads.size();
  lck_unlock(lck);

  return result;
}

unsigned int supla_abstract_asynctask_thread_pool::highest_number_of_threads(
    void) {
  lck_lock(lck);
  unsigned int result = _highest_number_of_threads;
  lck_unlock(lck);

  return result;
}

unsigned int supla_abstract_asynctask_thread_pool::overload_count(void) {
  lck_lock(lck);
  unsigned int result = _overload_count;
  lck_unlock(lck);

  return result;
}

unsigned int supla_abstract_asynctask_thread_pool::exec_count(void) {
  lck_lock(lck);
  unsigned int result = _exec_count;
  lck_unlock(lck);

  return result;
}

void supla_abstract_asynctask_thread_pool::terminate(void) {
  lck_lock(lck);
  terminated = true;

  for (auto it = threads.begin(); it != threads.end(); ++it) {
    sthread_terminate(*it, true);
  }

  lck_unlock(lck);
}

bool supla_abstract_asynctask_thread_pool::is_terminated(void) {
  lck_lock(lck);
  bool result = terminated;
  lck_unlock(lck);

  return result;
}

void supla_abstract_asynctask_thread_pool::hold(void) {
  lck_lock(lck);
  holded = true;
  lck_unlock(lck);
}

bool supla_abstract_asynctask_thread_pool::is_holded(void) {
  lck_lock(lck);
  bool result = holded;
  lck_unlock(lck);

  return result;
}

void supla_abstract_asynctask_thread_pool::unhold(void) {
  lck_lock(lck);
  holded = false;
  lck_unlock(lck);
}
