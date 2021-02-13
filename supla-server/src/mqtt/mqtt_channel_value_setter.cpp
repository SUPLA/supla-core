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

#include "mqtt_channel_value_setter.h"
#include "device.h"
#include "tools.h"

supla_mqtt_channel_value_setter::supla_mqtt_channel_value_setter(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_abstract_channel_value_setter(settings) {
  action_executor = NULL;
}

supla_mqtt_channel_value_setter::~supla_mqtt_channel_value_setter(void) {
  if (action_executor) {
    delete action_executor;
    action_executor = NULL;
  }
}

supla_action_executor *supla_mqtt_channel_value_setter::get_action_executor(
    void) {
  if (action_executor == NULL) {
    action_executor =
        new supla_action_executor(supla_user::find_by_suid(get_suid()),
                                  get_device_id(), get_channel_id());
  }

  return action_executor;
}

void supla_mqtt_channel_value_setter::set_on(bool on) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->set_on(on);
  }
}

void supla_mqtt_channel_value_setter::set_color(unsigned int color) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->set_color(color);
  }
}

void supla_mqtt_channel_value_setter::set_brightness(char brightness) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->set_brightness(brightness);
  }
}

void supla_mqtt_channel_value_setter::set_color_brightness(char brightness) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->set_color_brightness(brightness);
  }
}

void supla_mqtt_channel_value_setter::action_toggle(void) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->toggle();
  }
}

void supla_mqtt_channel_value_setter::action_shut(
    const char *closingPercentage) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->shut(closingPercentage);
  }
}

void supla_mqtt_channel_value_setter::action_reveal(void) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->reveal();
  }
}

void supla_mqtt_channel_value_setter::action_stop(void) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->stop();
  }
}

void supla_mqtt_channel_value_setter::action_open(void) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->open();
  }
}

void supla_mqtt_channel_value_setter::action_close(void) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->close();
  }
}

void supla_mqtt_channel_value_setter::action_open_close(void) {
  supla_action_executor *action = get_action_executor();
  if (action) {
    action->open_close();
  }
}
