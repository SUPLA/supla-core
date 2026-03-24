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

#ifndef VALUE_BASED_TRIGGER_CONDITION_RESULT_H_
#define VALUE_BASED_TRIGGER_CONDITION_RESULT_H_

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "caller.h"
#include "proto.h"
#include "user.h"

class supla_value_based_trigger;
class supla_vbt_condition_result {
 private:
  bool cnd_met;
  _supla_int64_t milliseconds_left;
  std::shared_ptr<supla_value_based_trigger> trigger;
  nlohmann::json template_data;
  supla_caller caller;
  supla_user *user;

 public:
  explicit supla_vbt_condition_result(bool cnd_met,
                                      _supla_int64_t milliseconds_left);
  virtual ~supla_vbt_condition_result(void);

  const std::shared_ptr<supla_value_based_trigger> &get_trigger(void) const;
  void set_trigger(const std::shared_ptr<supla_value_based_trigger> &trigger);

  const nlohmann::json &get_template_data(void) const;
  void set_template_data(const nlohmann::json &template_data);

  void set_caller(const supla_caller &caller);
  supla_caller get_caller(void) const;

  void set_user(supla_user *user);
  supla_user *get_user(void) const;

  bool is_condition_met(void);
  bool is_condition_met(_supla_int64_t *milliseconds_left);
};

#endif /* VALUE_BASED_TRIGGER_CONDITION_RESULT_H_ */
