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

#ifndef VALUE_BASED_TRIGGERS_H_
#define VALUE_BASED_TRIGGERS_H_

#include <memory>
#include <vector>

#include "device/value/channel_value.h"
#include "user/user.h"
#include "vbt/value_based_trigger.h"

class supla_value_based_triggers {
 private:
  supla_user *user;
  void *lck;
  std::vector<std::shared_ptr<supla_value_based_trigger>> triggers;

 public:
  explicit supla_value_based_triggers(supla_user *user);
  virtual ~supla_value_based_triggers(void);

  void load(void);
  size_t get_count(void);

  virtual void on_channel_value_changed(
      const supla_caller &caller, int channel_id,
      supla_channel_value *old_value, supla_channel_value *new_value,
      supla_abstract_action_executor *action_executor,
      supla_abstract_channel_property_getter *property_getter);

  void on_channel_value_changed(const supla_caller &caller, int channel_id,
                                supla_channel_value *old_value,
                                supla_channel_value *new_value);
};

#endif /* VALUE_BASED_TRIGGERS_H_ */
