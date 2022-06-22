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

#include "asynctask_state.h"

class supla_asynctask_queue;
class supla_abstract_asynctask_thread_pool;
class supla_abstract_asynctask {
 private:
  void *lck;
  supla_asynctask_state state;
  long long delay_usec;
  struct timeval started_at;
  struct timeval execution_start_time;
  unsigned long long timeout_usec;
  supla_asynctask_queue *queue;
  supla_abstract_asynctask_thread_pool *pool;
  int priority;
  bool release_immediately;
  void init(supla_asynctask_queue *queue,
            supla_abstract_asynctask_thread_pool *pool, short priority,
            bool release_immediately);

 protected:
  friend class supla_abstract_asynctask_thread_pool;
  friend class supla_asynctask_queue;

  void lock(void);
  void unlock(void);
  void set_waiting(void);
  bool pick(void);
  virtual bool _execute(bool *execute_again) = 0;
  void execute(void);

 public:
  supla_abstract_asynctask(supla_asynctask_queue *queue,
                           supla_abstract_asynctask_thread_pool *pool,
                           short priority, bool release_immediately);
  supla_abstract_asynctask(supla_asynctask_queue *queue,
                           supla_abstract_asynctask_thread_pool *pool);
  virtual ~supla_abstract_asynctask(void);
  supla_asynctask_queue *get_queue(void);
  supla_abstract_asynctask_thread_pool *get_pool(void);
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
  bool release_immediately_after_execution(void);
};

#endif /*ABSTRACT_ASYNCTASK_H_*/
