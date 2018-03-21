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

#ifndef ACTION_OPENCLOSE_H_
#define ACTION_OPENCLOSE_H_

#include "action.h"

class s_worker_action_openclose : public s_worker_action {
 private:
  bool doOpen;
  bool noSensor;

 protected:
  virtual void get_function_list(int list[FUNCTION_LIST_SIZE]);
  int retry_limit(void);
  int waiting_time_to_retry(void);
  int waiting_time_to_check(void);
  bool check_result();
  void do_action();
  bool garage_group(void);
  bool check_before_start(void);
  bool no_sensor(void);

 public:
  s_worker_action_openclose(s_worker *worker, bool doOpen);
};

class s_worker_action_open : public s_worker_action_openclose {
 protected:
  virtual void get_function_list(int list[FUNCTION_LIST_SIZE]);

 public:
  explicit s_worker_action_open(s_worker *worker);
};

class s_worker_action_close : public s_worker_action_openclose {
 public:
  explicit s_worker_action_close(s_worker *worker);
};

#endif /*ACTION_OPENCLOSE_H_*/
