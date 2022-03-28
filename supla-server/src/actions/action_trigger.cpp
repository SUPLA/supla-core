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

#include "action_trigger.h"

#include "user.h"

supla_action_trigger::supla_action_trigger(
    supla_abstract_action_executor *aexec, action_trigger_config *config,
    supla_abstract_value_getter *value_getter) {
  this->aexec = aexec;
  this->config = config;
  this->value_getter = value_getter;
}

supla_action_trigger::~supla_action_trigger(void) {}

void supla_action_trigger::execute_actions(int at_channel_id, int user_id,
                                           unsigned int caps) {
  if (!aexec || !config) {
    return;
  }

  caps = config->get_active_actions() & caps;
  unsigned char count = sizeof(caps) * 8;
  for (short a = 0; a < count; a++) {
    unsigned int cap = caps & (1 << a);
    if (!cap) {
      continue;
    }

    config->set_active_cap(cap);
    aexec->execute_action(supla_caller(ctActionTrigger, at_channel_id), user_id,
                          config, value_getter);
  }
}
