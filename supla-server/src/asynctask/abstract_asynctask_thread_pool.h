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

class supla_asynctask_queue;
class supla_abstract_asynctask_thread_pool {
 private:
  void *lck;
  supla_asynctask_queue *queue;
  bool terminated;

 protected:
  friend class supla_asynctask_queue;
  void execution_request(void);
  void terminate(void);

 public:
  explicit supla_abstract_asynctask_thread_pool(supla_asynctask_queue *queue);
  virtual ~supla_abstract_asynctask_thread_pool(void);
  virtual int thread_count_limit(void) = 0;
  unsigned int thread_count(void);
  bool is_terminated(void);
};

#endif /*ASYNCTASK_THREAD_POOL_H_*/
