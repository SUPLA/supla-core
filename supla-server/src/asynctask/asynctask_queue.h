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

#ifndef ASYNCTASK_QUEUE_H_
#define ASYNCTASK_QUEUE_H_

#include <functional>
#include <vector>

#include "abstract_asynctask.h"
#include "abstract_asynctask_observer.h"
#include "abstract_asynctask_search_condition.h"
#include "eh.h"

class supla_asynctask_queue {
 private:
  static supla_asynctask_queue *_global_instance;
  void *lck;
  void *observer_lck;
  void *thread;
  TEventHandler *eh;
  unsigned long long last_iterate_time_sec;
  std::vector<supla_abstract_asynctask_observer *> observers;

  std::vector<supla_abstract_asynctask *> tasks;
  std::vector<supla_abstract_asynctask_thread_pool *> pools;
  void iterate(void);
  void release_tasks(void);
  void release_pools(void);
  static void loop(void *_queue, void *q_sthread);
  supla_abstract_asynctask *find_task(
      supla_abstract_asynctask_search_condition *cnd);

 protected:
  friend class supla_abstract_asynctask;
  friend class supla_abstract_asynctask_thread_pool;

  bool task_exists(supla_abstract_asynctask *task);
  void add_task(supla_abstract_asynctask *task);
  void remove_task(supla_abstract_asynctask *task);
  bool pool_exists(supla_abstract_asynctask_thread_pool *pool);
  void register_pool(supla_abstract_asynctask_thread_pool *pool);
  void unregister_pool(supla_abstract_asynctask_thread_pool *pool);
  void on_task_started(supla_abstract_asynctask *task);
  void on_task_finished(supla_abstract_asynctask *task);

  supla_abstract_asynctask *pick(supla_abstract_asynctask_thread_pool *pool);

 public:
  supla_asynctask_queue(void);
  virtual ~supla_asynctask_queue(void);
  static supla_asynctask_queue *global_instance(void);
  static void global_instance_release(void);

  unsigned int total_count(void);
  unsigned int waiting_count(void);
  unsigned int thread_count(void);
  unsigned int pool_count(void);
  void raise_event(void);
  bool get_task_state(supla_asynctask_state *state,
                      supla_abstract_asynctask_search_condition *cnd);
  unsigned int get_task_count(supla_abstract_asynctask_search_condition *cnd);
  bool task_exists(supla_abstract_asynctask_search_condition *cnd);
  void cancel_tasks(supla_abstract_asynctask_search_condition *cnd);
  void add_observer(supla_abstract_asynctask_observer *observer);
  void remove_observer(supla_abstract_asynctask_observer *observer);
  bool access_task(supla_abstract_asynctask_search_condition *cnd,
                   std::function<void(supla_abstract_asynctask *)> on_task);
  void log_stuck_warning(void);
};

#endif /*ASYNCTASK_QUEUE_H_*/
