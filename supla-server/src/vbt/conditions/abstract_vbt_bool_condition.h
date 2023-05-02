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

#ifndef ABSTRACT_VALUE_BASED_TRIGGER_BOOL_CONDITION_H_
#define ABSTRACT_VALUE_BASED_TRIGGER_BOOL_CONDITION_H_

#include <string>

#include "device/value/channel_value.h"
#include "vbt/conditions/abstract_vbt_condition.h"

enum _vbt_bool_e { vbt_bool_not_set, vbt_bool_false, vbt_bool_true };

class supla_abstract_vbt_bool_condition : public supla_abstract_vbt_condition {
 private:
  _vbt_bool_e expected;

 protected:
  virtual bool convert(supla_channel_value *value, _vbt_bool_e *converted) = 0;
  void set_expected(_vbt_bool_e expected);
  _vbt_bool_e get_expected(void);

 public:
  supla_abstract_vbt_bool_condition(void);
  explicit supla_abstract_vbt_bool_condition(cJSON *json);
  virtual ~supla_abstract_vbt_bool_condition(void);

  virtual bool is_condition_met(supla_channel_value *old_value,
                                supla_channel_value *new_value);
};

#endif /* ABSTRACT_VALUE_BASED_TRIGGER_BOOL_CONDITION_H_ */
