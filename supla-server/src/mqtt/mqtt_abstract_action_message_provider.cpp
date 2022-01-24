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

#include "mqtt_abstract_action_message_provider.h"

#include "log.h"

supla_mqtt_abstract_action_message_provider::
    supla_mqtt_abstract_action_message_provider(void)
    : supla_mqtt_message_provider() {
  this->action = 0;
  this->user_id = 0;
  this->device_id = 0;
  this->channel_id = 0;
  this->user_suid = NULL;
}

supla_mqtt_abstract_action_message_provider::
    ~supla_mqtt_abstract_action_message_provider(void) {}

bool supla_mqtt_abstract_action_message_provider::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index != 0 || user_suid == NULL || !device_id || !channel_id || !action) {
    return false;
  }

  char topic[30] = {};

  switch (action) {
    case SUPLA_ACTION_CAP_TURN_ON:
      snprintf(topic, sizeof(topic), "turn_on");
      break;
    case SUPLA_ACTION_CAP_TURN_OFF:
      snprintf(topic, sizeof(topic), "turn_off");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x1:
      snprintf(topic, sizeof(topic), "toggle_x1");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x2:
      snprintf(topic, sizeof(topic), "toggle_x2");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x3:
      snprintf(topic, sizeof(topic), "toggle_x3");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x4:
      snprintf(topic, sizeof(topic), "toggle_x4");
      break;
    case SUPLA_ACTION_CAP_TOGGLE_x5:
      snprintf(topic, sizeof(topic), "toggle_x5");
      break;
    case SUPLA_ACTION_CAP_HOLD:
      snprintf(topic, sizeof(topic), "hold");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x1:
      snprintf(topic, sizeof(topic), "press_x1");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x2:
      snprintf(topic, sizeof(topic), "press_x2");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x3:
      snprintf(topic, sizeof(topic), "press_x3");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x4:
      snprintf(topic, sizeof(topic), "press_x4");
      break;
    case SUPLA_ACTION_CAP_SHORT_PRESS_x5:
      snprintf(topic, sizeof(topic), "press_x5");
      break;
    default:
      return false;
  }

  return create_message(
      topic_prefix, user_suid, topic_name, message, message_size, topic, false,
      "devices/%i/channels/%i/action/%s", device_id, channel_id, topic);
}

void supla_mqtt_abstract_action_message_provider::set_action(
    int user_id, int device_id, int channel_id, unsigned int action) {
  if (this->user_id != user_id) {
    user_suid = (char *)_get_user_suid(user_id);
  }

  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;
  this->action = action;
}
