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

#ifndef ASYNCTASK_MOCK_H_
#define ASYNCTASK_MOCK_H_

#include <sys/time.h>
#include "abstract_asynctask.h"

class AsyncTaskMock : public supla_abstract_asynctask {
 private:
  unsigned int job_time_usec;
  unsigned short job_count_left;
  unsigned int _exec_count;
  bool _result;
  struct timeval init_time;
  struct timeval exec_time;
  int tag;

  void mock_init(void);

 protected:
  virtual bool _execute(bool *execute_again);

 public:
  AsyncTaskMock(supla_asynctask_queue *queue,
                supla_abstract_asynctask_thread_pool *pool, short priority,
                bool release_immediately);
  AsyncTaskMock(supla_asynctask_queue *queue,
                supla_abstract_asynctask_thread_pool *pool);
  virtual ~AsyncTaskMock(void);
  void set_job_time_usec(unsigned int job_time_usec);
  void set_job_count_left(unsigned short job_count_left);
  unsigned short get_job_count_left(void);
  void set_result(bool result);
  unsigned int exec_count(void);
  void set_waiting(void);
  long long exec_delay_usec(void);
  long long exec_time_since(struct timeval *time);
  void set_retry_left(int retry_left);
  void set_tag(int tag);
  int get_tag(void);
};

#endif /*ASYNCTASK_MOCK_H_*/
