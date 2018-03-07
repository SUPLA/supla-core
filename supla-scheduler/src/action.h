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

#ifndef ACTION_H_
#define ACTION_H_

#include "proto.h"
#include "worker.h"

#define FUNCTION_LIST_SIZE 10

class s_worker_action {
 private:
  bool check_function_allowed(void);
  void _check_result(void);
  int _retry_limit(void);
  int _waiting_time_to_retry(void);
  int _waiting_time_to_check(void);

 protected:
  s_worker *worker;

  virtual void get_function_list(int list[FUNCTION_LIST_SIZE]) = 0;
  virtual int retry_limit(void) = 0;
  virtual int waiting_time_to_retry(void) = 0;  // return seconds
  virtual int waiting_time_to_check(void) = 0;  // return seconds
  virtual void do_action(void) = 0;
  virtual bool check_result(void) = 0;

 public:
  explicit s_worker_action(s_worker *worker);
  virtual ~s_worker_action();
  void execute(void);
};

#endif /*ACTION_H_*/
