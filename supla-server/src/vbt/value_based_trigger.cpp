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

supla_value_based_trigger::supla_value_based_trigger(
    int id, int channel_id, int func, _subjectType_e subject_type,
    int subject_id, supla_action_config *action_config) {
  this->id = id;
  this->channel_id = channel_id;
  this->subject_type = subject_type;
  this->subject_id = subject_id;
  this->cnd = nullptr;
  this->action_config = action_config;
}

supla_value_based_trigger::~supla_value_based_trigger(void) {
  if (action_config) {
    delete action_config;
  }

  if (cnd) {
    delete cnd;
  }
}

supla_vbt_condition_result supla_value_based_trigger::are_conditions_met(
    int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value) {
  bool cnd_met = channel_id == this->channel_id && cnd &&
                 cnd->is_condition_met(old_value, new_value);
  supla_vbt_condition_result result(cnd_met);
  result.set_replacement_map(new_value->get_replacement_map());
  return result;
}

void supla_value_based_trigger::fire(
    const supla_caller &caller, int user_id,
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter,
    supla_pn_dispatcher *pn_dispatcher,
    const std::map<std::string, std::string> &replacement_map) {
  if (subject_type == stPushNotifiction) {
    if (subject_id) {
      pn_dispatcher->send_push(user_id, subject_id, replacement_map);
    }
    return;
  }

  if (action_config) {
    action_executor->execute_action(caller, user_id, action_config,
                                    property_getter);
  }
}

int supla_value_based_trigger::get_id(void) { return id; }

int supla_value_based_trigger::get_channel_id(void) { return channel_id; }

_subjectType_e supla_value_based_trigger::get_subject_type(void) {
  return subject_type;
}

int supla_value_based_trigger::get_subject_id(void) { return subject_id; }

const supla_action_config *supla_value_based_trigger::get_action_config(void) {
  return action_config;
}

const supla_abstract_vbt_condition *supla_value_based_trigger::get_condition(
    void) {
  return cnd;
}

void supla_value_based_trigger::update(supla_value_based_trigger *trigger) {
  if (this->channel_id != trigger->channel_id ||
      this->subject_type != trigger->subject_type ||
      this->subject_id != trigger->subject_id || !this->cnd || !trigger->cnd ||
      !this->cnd->equal(trigger->cnd)) {
    if (this->cnd) {
      delete this->cnd;
      this->cnd = nullptr;
    }

    if (trigger->cnd) {
      this->cnd = trigger->cnd->copy();
    }
  }

  if (action_config) {
    delete action_config;
    action_config = nullptr;
  }

  if (trigger->action_config) {
    action_config = new supla_action_config();
    *this->action_config = *trigger->action_config;
  }

  this->channel_id = trigger->channel_id;
  this->subject_type = trigger->subject_type;
  this->subject_id = trigger->subject_id;
}
