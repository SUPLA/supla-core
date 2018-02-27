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

#ifndef QUEUE_H_
#define QUEUE_H_

#include "database.h"
#include "eh.h"

class queue {
 private:
  void *user;
  void *q_sthread;

  TEventHandler *loop_eh;
  void *lck;
  void *workers_thread_arr;
  void *s_exec_arr;

  int max_workers;
  int max_job_per_second;

  int job_counter;
  int total_fetch_count;
  int wait_for_fetch_counter;

  struct timeval last_get;
  struct timeval timer;
  struct timeval timer_one_min;

  void new_worker(void);
  bool wait_for_fetch(void);

 protected:
  database *db;

 public:
  queue(void *user, void *q_sthread);
  ~queue();

  void loop(void);

  void load(void);
  void raise_loop_event(void);

  void set_overdue_result(void);
  void set_zombie_result(void);

  s_exec_t get_job(void);
  void mark_fetched();

  bool limit_exceeded();

  void print_statistics(void);
};

void queue_loop(void *user, void *q_sthread);

#endif /* QUEUE_H_ */
