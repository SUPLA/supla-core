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
#include "asynctask_queue.h"
#include "lck.h"

supla_abstract_asynctask::supla_abstract_asynctask(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    short priority) {
  assert(queue);
  assert(pool);

  this->lck = lck_init();
  this->state = STA_STATE_INIT;
  this->delay_usec = 0;
  this->start_time.tv_sec = 0;
  this->start_time.tv_usec = 0;
  this->timeout_usec = 0;
  this->queue = queue;
  this->pool = pool;
  this->priority = priority;

  queue->add_task(this);
}

supla_abstract_asynctask::~supla_abstract_asynctask(void) {
  queue->remove_task(this);
  lck_free(lck);
}

void supla_abstract_asynctask::lock(void) { lck_lock(lck); }

void supla_abstract_asynctask::unlock(void) { lck_unlock(lck); }

supla_asynctask_queue *supla_abstract_asynctask::get_queue(void) {
  return queue;
}

supla_abstract_asynctask_thread_pool *supla_abstract_asynctask::get_pool(void) {
  return pool;
}

short supla_abstract_asynctask::get_priority(void) { return priority; }

async_task_state supla_abstract_asynctask::get_state(void) {
  lock();
  async_task_state result = state;
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
  gettimeofday(&start_time, NULL);
  this->delay_usec = delay_usec;

  if (delay_usec > 0) {
    start_time.tv_sec += delay_usec / 1000000;
    start_time.tv_usec += delay_usec % 1000000;
  }

  unlock();
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
  if (now->tv_sec <= start_time.tv_sec) {
    result = (start_time.tv_sec * (long long)1000000 + start_time.tv_usec) -
             (now->tv_sec * (long long)1000000 + now->tv_usec);
  }
  unlock();

  return result;
}

void supla_abstract_asynctask::set_waiting(void) {
  lock();
  if (state == STA_STATE_INIT) {
    state = STA_STATE_WAITING;
  }
  unlock();
}

bool supla_abstract_asynctask::pick(void) {
  lock();
  bool result = state == STA_STATE_WAITING;
  if (result) {
    state = STA_STATE_PICKED;
  }
  unlock();

  return result;
}

void supla_abstract_asynctask::execute(void) {
  lock();
  bool exec_allowed = state == STA_STATE_PICKED;

  if (exec_allowed && timeout_usec) {
    long long time_left = time_left_usec(NULL);
    if (time_left < 0) {
      time_left *= -1;
      if ((long long unsigned)time_left >= timeout_usec) {
        state = STA_STATE_TIMEOUT;
        exec_allowed = false;
      }
    }
  }

  if (exec_allowed) {
    state = STA_STATE_EXECUTING;
  }
  unlock();

  if (!exec_allowed) {
    return;
  }

  bool exec_again = false;
  bool result = _execute(&exec_again);

  lock();
  if (state != STA_STATE_CANCELED) {
    if (exec_again) {
      state = STA_STATE_WAITING;
    } else if (result) {
      state = STA_STATE_SUCCESS;
    } else {
      state = STA_STATE_FAILURE;
    }
  }
  unlock();
}

void supla_abstract_asynctask::cancel(void) {
  lock();
  state = STA_STATE_CANCELED;
  unlock();
}

bool supla_abstract_asynctask::is_finished(void) {
  lock();
  bool result = false;

  switch (state) {
    case STA_STATE_SUCCESS:
    case STA_STATE_FAILURE:
    case STA_STATE_TIMEOUT:
    case STA_STATE_CANCELED:
      result = true;
      break;
    default:
      break;
  }

  unlock();

  return result;
}
