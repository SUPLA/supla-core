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

#ifndef ABSTRACT_ASYNCTASK_H_
#define ABSTRACT_ASYNCTASK_H_

#include <sys/time.h>

#include <list>
#include <memory>

#include "asynctask_state.h"
#include "asynctask_thread_bucket.h"

class supla_asynctask_queue;
class supla_abstract_asynctask_thread_pool;
class supla_abstract_asynctask {
 private:
  void *lck;
  bool observable;
  bool observer_notified;
  supla_asynctask_state state;
  long long delay_usec;
  struct timeval started_at;
  struct timeval execution_start_time;
  unsigned long long timeout_usec;
  supla_asynctask_queue *queue;
  supla_abstract_asynctask_thread_pool *pool;
  int priority;

 protected:
  friend class supla_abstract_asynctask_thread_pool;
  friend class supla_asynctask_queue;

  void lock(void);
  void unlock(void);

  bool pick(void);
  virtual bool _execute(bool *execute_again,
                        supla_asynctask_thread_bucket *bucket) = 0;
  void execute(supla_asynctask_thread_bucket *bucket);
  void set_observable(void);  // This method should only be called in the
                              // constructor. Calling it results in calling the
                              // on_asynctask_started method in the observer
  void on_task_finished(void);
  virtual void on_timeout(long long unsigned usec_after_timeout);

 public:
  supla_abstract_asynctask(supla_asynctask_queue *queue,
                           supla_abstract_asynctask_thread_pool *pool);
  virtual ~supla_abstract_asynctask(void);
  std::shared_ptr<supla_abstract_asynctask> start(void);
  supla_asynctask_queue *get_queue(void);
  supla_abstract_asynctask_thread_pool *get_pool(void);
  void set_priority(short priority);  // Priority can only be set before start
  short get_priority(void);
  struct timeval get_started_at(void);
  long long time_left_usec(struct timeval *now);
  supla_asynctask_state get_state(void);
  long long get_delay_usec(void);
  void set_delay_usec(long long delay_usec);
  void set_timeout(unsigned long long timeout_usec);
  unsigned long long get_timeout(void);
  void cancel(void);
  bool is_finished(void);
  bool is_observable(void);
};

#endif /*ABSTRACT_ASYNCTASK_H_*/
