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

#include "value_based_trigger.h"

#include "vbt/vbt_on_change_condition.h"

supla_value_based_trigger::supla_value_based_trigger(
    int id, int channel_id, _subjectType_e subject_type, int subject_id,
    supla_action_config *action_config, const char *conditions) {
  this->id = id;
  this->channel_id = channel_id;
  this->subject_type = subject_type;
  this->subject_id = subject_id;
  this->action_config = action_config;

  cJSON *json = nullptr;

  if (conditions && conditions[0] != 0) {
    json = cJSON_Parse(conditions);

    if (json) {
      on_change_cnd.apply_json_config(json);
      cJSON_Delete(json);
    }
  }
}

supla_value_based_trigger::~supla_value_based_trigger(void) {
  if (action_config) {
    delete action_config;
  }
}

supla_vbt_condition_result supla_value_based_trigger::are_conditions_met(
    int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value) {
  bool cnd_met = channel_id == this->channel_id &&
                 on_change_cnd.is_condition_met(old_value, new_value);
  supla_vbt_condition_result result(cnd_met);
  result.set_replacement_map(new_value->get_replacement_map());
  return result;
}

void supla_value_based_trigger::fire(
    const supla_caller &caller, int user_id,
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter,
    const std::map<std::string, std::string> &replacement_map) {
  if (action_config) {
    action_executor->execute_action(caller, user_id, action_config,
                                    property_getter);
  }
}

int supla_value_based_trigger::get_id(void) { return id; }

bool supla_value_based_trigger::equal(
    const supla_value_based_trigger &trigger) const {
  return id == trigger.id && channel_id == trigger.channel_id &&
         subject_type == trigger.subject_type &&
         subject_id == trigger.subject_id &&
         on_change_cnd == trigger.on_change_cnd &&
         ((!action_config && !trigger.action_config) ||
          (action_config && trigger.action_config &&
           *action_config == *trigger.action_config));
}

bool supla_value_based_trigger::operator==(
    const supla_value_based_trigger &trigger) const {
  return equal(trigger);
}

bool supla_value_based_trigger::operator!=(
    const supla_value_based_trigger &trigger) const {
  return !equal(trigger);
}

bool supla_value_based_trigger::operator==(
    const supla_value_based_trigger *trigger) const {
  return *this == *trigger;
}

bool supla_value_based_trigger::operator!=(
    const supla_value_based_trigger *trigger) const {
  return *this != *trigger;
}
