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
    int channel_id, int func, _subjectType_e subject_type, int subject_id) {
  this->channel_id = func;
  this->subject_type = subject_type;
  this->subject_id = subject_id;
  this->cnd = nullptr;
}

supla_value_based_trigger::~supla_value_based_trigger(void) {}

bool supla_value_based_trigger::are_conditions_met(
    int channel_id, supla_channel_value *old_value,
    supla_channel_value *new_value) {
  return channel_id == this->channel_id && cnd &&
         cnd->is_condition_met(old_value, new_value);
}

void supla_value_based_trigger::fire(
    supla_abstract_action_executor *action_executor,
    supla_pn_dispatcher *pn_dispatcher) {}
