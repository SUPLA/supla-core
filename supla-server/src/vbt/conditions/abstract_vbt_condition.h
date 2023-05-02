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

#ifndef ABSTRACT_VALUE_BASED_TRIGGER_CONDITION_H_
#define ABSTRACT_VALUE_BASED_TRIGGER_CONDITION_H_

#include <string>

#include "device/value/channel_value.h"

class supla_abstract_vbt_condition {
 public:
  explicit supla_abstract_vbt_condition(const std::string &conditions);
  virtual ~supla_abstract_vbt_condition(void);

  virtual bool equal(supla_abstract_vbt_condition *cnd) = 0;
  virtual supla_abstract_vbt_condition *copy(void) = 0; // NOLINT

  virtual bool is_condition_met(supla_channel_value *old_value,
                                supla_channel_value *new_value) = 0;
};

#endif /* ABSTRACT_VALUE_BASED_TRIGGER_CONDITION_H_ */
