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

#ifndef ABSTRACT_RS_ACTION_H_
#define ABSTRACT_RS_ACTION_H_

#include "action.h"

class s_abstract_rs_action : public s_worker_action {
 protected:
  virtual bool is_action_allowed(void);
  int try_limit(void);
  int waiting_time_to_retry(void);
  int waiting_time_to_check(void);
  bool result_success(int *fail_result_code);
  virtual bool get_expected_value(char *expected_value) = 0;

 public:
  bool parse_percentage(char *percent);
  explicit s_abstract_rs_action(s_abstract_worker *worker);
};

#endif /* ABSTRACT_RS_ACTION_H_ */
