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

#ifndef ASYNCTASK_THREAD_POOL_H_
#define ASYNCTASK_THREAD_POOL_H_

#include <sys/time.h>

#include <functional>
#include <string>
#include <vector>

class supla_asynctask_queue;
class supla_abstract_asynctask;
class supla_asynctask_thread_bucket;
class supla_abstract_asynctask_thread_pool {
 private:
  void *lck;
  supla_asynctask_thread_bucket *bucket;
  std::vector<void *> threads;
  std::vector<supla_abstract_asynctask *> requests;
  std::vector<supla_asynctask_thread_bucket *> buckets;
  supla_asynctask_queue *queue;
  struct timeval warinig_time;
  unsigned int _overload_count;
  unsigned int _exec_count;
  unsigned int _highest_number_of_threads;
  bool holded;
  bool terminated;
  static void _execute(void *_pool, void *sthread);
  static void _on_thread_finish(void *_pool, void *sthread);
  void execute(void *sthread);
  void on_thread_finish(void *sthread);
  void add_bucket(supla_asynctask_thread_bucket *bucket);
  void remove_bucket(supla_asynctask_thread_bucket *bucket);

 protected:
  friend class supla_asynctask_queue;

  virtual supla_asynctask_thread_bucket *get_bucket(void) = 0;

  void execution_request(supla_abstract_asynctask *task);
  void remove_task(supla_abstract_asynctask *task);
  virtual int tasks_per_thread(void);
  virtual bool should_keep_alive(unsigned long long usec_since_last_exec,
                                 size_t thread_count);

 public:
  explicit supla_abstract_asynctask_thread_pool(supla_asynctask_queue *queue);
  virtual ~supla_abstract_asynctask_thread_pool(void);

  virtual unsigned int thread_count_limit(void) = 0;
  virtual std::string pool_name(void) = 0;
  unsigned int thread_count(void);
  unsigned int highest_number_of_threads(void);
  unsigned int overload_count(void);
  unsigned int exec_count(void);
  void terminate(void);
  bool is_terminated(void);
  void hold(void);
  bool is_holded(void);
  void unhold(void);
};

#endif /*ASYNCTASK_THREAD_POOL_H_*/
