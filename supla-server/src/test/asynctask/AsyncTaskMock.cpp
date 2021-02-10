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

#include "AsyncTaskMock.h"
#include <unistd.h>

AsyncTaskMock::AsyncTaskMock(supla_asynctask_queue *queue,
                             supla_abstract_asynctask_thread_pool *pool,
                             short priority, bool release_immediately)
    : supla_abstract_asynctask(queue, pool, priority, release_immediately) {
  this->job_time_usec = 0;
  this->job_count_left = 1;
  this->_exec_count = 0;
  this->_result = false;
  this->exec_time.tv_sec = 0;
  this->exec_time.tv_usec = 0;

  gettimeofday(&this->init_time, NULL);
}

AsyncTaskMock::~AsyncTaskMock(void) {}

bool AsyncTaskMock::_execute(bool *execute_again) {
  lock();
  _exec_count++;
  gettimeofday(&this->exec_time, NULL);
  unsigned int time = job_time_usec;
  bool result = _result;
  job_count_left--;
  if (job_count_left > 0) {
    *execute_again = true;
  }
  unlock();

  if (time) {
    usleep(time);
  }

  return result;
}

void AsyncTaskMock::set_job_time_usec(unsigned int job_time_usec) {
  lock();
  this->job_time_usec = job_time_usec;
  unlock();
}

void AsyncTaskMock::set_job_count_left(unsigned short job_count_left) {
  if (job_count_left < 1) {
    return;
  }

  lock();
  this->job_count_left = job_count_left;
  unlock();
}

void AsyncTaskMock::set_result(bool result) {
  lock();
  _result = result;
  unlock();
}

void AsyncTaskMock::set_waiting(void) {
  supla_abstract_asynctask::set_waiting();
}

long long AsyncTaskMock::exec_delay_usec(void) {
  lock();
  long long result =
      (init_time.tv_sec * (long long)1000000 + init_time.tv_usec) -
      (exec_time.tv_sec * (long long)1000000 + exec_time.tv_usec);
  unlock();

  return result;
}
