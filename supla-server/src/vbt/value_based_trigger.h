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

#ifndef VALUE_BASED_TRIGGER_H_
#define VALUE_BASED_TRIGGER_H_

#include <map>
#include <string>
#include <vector>

#include "actions/abstract_action_executor.h"
#include "actions/action_config.h"
#include "push/pn_dispatcher.h"
#include "user/user.h"
#include "vbt/conditions/abstract_vbt_condition.h"
#include "vbt/vbt_condition_result.h"

class supla_value_based_trigger {
 private:
  int id;
  int channel_id;
  _subjectType_e subject_type;
  int subject_id;
  supla_abstract_vbt_condition *cnd;
  supla_action_config *action_config;

 public:
  explicit supla_value_based_trigger(int id, int channel_id, int func,
                                     _subjectType_e subject_type,
                                     int subject_id,
                                     supla_action_config *action_config);
  virtual ~supla_value_based_trigger(void);

  int get_id(void);
  int get_channel_id(void);
  _subjectType_e get_subject_type(void);
  int get_subject_id(void);
  const supla_action_config *get_action_config(void);
  const supla_abstract_vbt_condition *get_condition(void);

  void update(supla_value_based_trigger *trigger);

  supla_vbt_condition_result are_conditions_met(int channel_id,
                                                supla_channel_value *old_value,
                                                supla_channel_value *new_value);

  void fire(const supla_caller &caller, int user_id,
            supla_abstract_action_executor *action_executor,
            supla_abstract_channel_property_getter *property_getter,
            supla_pn_dispatcher *pn_dispatcher,
            const std::map<std::string, std::string> &replacement_map);
};

#endif /* VALUE_BASED_TRIGGER_H_ */
