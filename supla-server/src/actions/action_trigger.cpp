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
    supla_abstract_channel_action_executor *ca_exec,
    action_trigger_config *config, supla_abstract_device_finder *dev_finder) {
  this->ca_exec = ca_exec;
  this->config = config;
  this->dev_finder = dev_finder;
}

supla_action_trigger::~supla_action_trigger(void) {}

void supla_action_trigger::execute_actions(int user_id, unsigned int caps) {
  if (!ca_exec || !config) {
    return;
  }

  caps = config->get_active_actions() & caps;
  unsigned char count = sizeof(caps) * 8;
  for (short a = 0; a < count; a++) {
    unsigned int cap = caps & (1 << a);
    if (!cap) {
      continue;
    }

    _at_config_action_t action = config->get_action_assigned_to_capability(cap);

    if (!action.actionId || !action.subjectId) {
      continue;
    }

    if (action.channelGroup) {
    } else {
      int device_id =
          dev_finder ? dev_finder->find_device_id(user_id, action.subjectId)
                     : 0;

      if (!device_id) {
        continue;
      }

      ca_exec->set_channel_id(user_id, device_id, action.subjectId);

      switch (action.actionId) {
        case ACTION_OPEN:
          ca_exec->open();
          break;
        case ACTION_CLOSE:
          ca_exec->close();
          break;
        case ACTION_SHUT:
          ca_exec->shut(NULL);
          break;
        case ACTION_REVEAL:
          ca_exec->reveal();
          break;
        case ACTION_REVEAL_PARTIALLY: {
          char percentage = config->get_percentage(cap);
          if (percentage > -1) {
            percentage = 100 - percentage;
            ca_exec->shut(&percentage);
          }
        } break;
        case ACTION_TURN_ON:
          ca_exec->set_on(true);
          break;
        case ACTION_TURN_OFF:
          ca_exec->set_on(false);
          break;
        case ACTION_SET_RGBW_PARAMETERS: {
          _at_config_rgbw_t rgbw = config->get_rgbw(cap);
          if (rgbw.brightness > -1 || rgbw.color_brightness > -1 ||
              rgbw.color) {
            ca_exec->set_rgbw(
                rgbw.color ? &rgbw.color : NULL,
                rgbw.color_brightness > -1 ? &rgbw.color_brightness : NULL,
                rgbw.brightness > -1 ? &rgbw.brightness : NULL);
          }
        } break;
        case ACTION_OPEN_CLOSE:
          ca_exec->open_close();
          break;
        case ACTION_STOP:
          ca_exec->stop();
          break;
        case ACTION_TOGGLE:
          ca_exec->toggle();
          break;
      }
    }
  }
}