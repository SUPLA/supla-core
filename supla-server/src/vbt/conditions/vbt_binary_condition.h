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

#ifndef VALUE_BASED_TRIGGER_BINARY_CONDITION_H_
#define VALUE_BASED_TRIGGER_BINARY_CONDITION_H_

#include "vbt/conditions/abstract_vbt_bool_condition.h"

class supla_vbt_binary_condition : public supla_abstract_vbt_bool_condition {
 protected:
  virtual bool convert(supla_channel_value *value, _vbt_bool_e *converted);

 public:
  supla_vbt_binary_condition(void);
  explicit supla_vbt_binary_condition(cJSON *json);
  virtual ~supla_vbt_binary_condition(void);

  virtual bool equal(supla_abstract_vbt_condition *cnd);
  virtual supla_abstract_vbt_condition *copy(void);  // NOLINT
  static bool is_function_supported(int func);
};

#endif /* VALUE_BASED_TRIGGER_BINARY_CONDITION_H_ */
