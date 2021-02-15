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

#include "action_gate_openclose_search_condition.h"
#include "action_gate_openclose.h"

supla_action_gate_openclose_search_condition::
    supla_action_gate_openclose_search_condition(supla_abstract_asynctask *skip,
                                                 int user_id, int device_id,
                                                 int channel_id) {
  this->skip = skip;
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;
}

bool supla_action_gate_openclose_search_condition::condition_met(
    supla_abstract_asynctask *task) {
  if (task == skip) {
    return false;
  }

  supla_action_gate_openclose *oc_task =
      dynamic_cast<supla_action_gate_openclose *>(task);
  return oc_task && oc_task->get_user_id() == user_id &&
         oc_task->get_device_id() == device_id &&
         oc_task->get_channel_id() == channel_id;
}
