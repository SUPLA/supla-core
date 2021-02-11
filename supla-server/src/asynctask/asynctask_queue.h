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

#include <vector>
#include "abstract_asynctask.h"
#include "eh.h"

class supla_asynctask_queue {
 private:
  void *lck;
  void *thread;
  TEventHandler *eh;

  std::vector<supla_abstract_asynctask *> tasks;
  std::vector<supla_abstract_asynctask_thread_pool *> pools;
  void iterate(void);
  void release_tasks(void);
  void release_pools(void);
  static void loop(void *_queue, void *q_sthread);

 protected:
  friend class supla_abstract_asynctask;
  friend class supla_abstract_asynctask_thread_pool;

  bool task_exists(supla_abstract_asynctask *task);
  void add_task(supla_abstract_asynctask *task);
  void remove_task(supla_abstract_asynctask *task);
  bool pool_exists(supla_abstract_asynctask_thread_pool *pool);
  void register_pool(supla_abstract_asynctask_thread_pool *pool);
  void unregister_pool(supla_abstract_asynctask_thread_pool *pool);

  supla_abstract_asynctask *pick(supla_abstract_asynctask_thread_pool *pool);

 public:
  supla_asynctask_queue(void);
  virtual ~supla_asynctask_queue(void);
  unsigned int total_count(void);
  unsigned int waiting_count(void);
  unsigned int thread_count(void);
  unsigned int pool_count(void);
  void raise_event(void);
};

#endif /*ASYNCTASK_QUEUE_H_*/
