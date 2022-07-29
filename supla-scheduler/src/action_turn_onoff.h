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

#ifndef ACTION_TURN_ONOFF_H_
#define ACTION_TURN_ONOFF_H_

#include "action.h"

class s_worker_action_turn_onoff : public s_worker_action {
 private:
  bool setOn;

 protected:
  virtual bool is_action_allowed(void);
  virtual int waiting_time_to_retry(void);
  virtual int waiting_time_to_check(void);
  virtual bool result_success(int *fail_result_code);
  virtual bool do_action();

 public:
  s_worker_action_turn_onoff(s_abstract_worker *worker, bool setOn);
  virtual int try_limit(void);
};

class s_worker_action_turn_on : public s_worker_action_turn_onoff {
 public:
  explicit s_worker_action_turn_on(s_abstract_worker *worker);
};

class s_worker_action_turn_off : public s_worker_action_turn_onoff {
 public:
  explicit s_worker_action_turn_off(s_abstract_worker *worker);
};

#endif /*ACTION_TURN_ONOFF_*/
