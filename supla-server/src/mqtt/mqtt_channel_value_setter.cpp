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

supla_mqtt_channel_value_setter::supla_mqtt_channel_value_setter(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_abstract_channel_value_setter(settings) {}

supla_mqtt_channel_value_setter::~supla_mqtt_channel_value_setter(void) {}

supla_user *supla_mqtt_channel_value_setter::get_user(void) {
  return supla_user::find_by_email(get_email());
}

supla_device *supla_mqtt_channel_value_setter::get_device(void) {
  supla_user *user = get_user();
  if (user) {
    return user->get_device(get_device_id());
  }

  return NULL;
}

void supla_mqtt_channel_value_setter::set_on(bool on) {
  supla_device *device = get_device();
  if (device) {
    device->set_on(0, get_channel_id(), 0, 0, on ? 1 : 0);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::set_color(unsigned int color) {
  supla_device *device = get_device();
  if (device) {
    device->set_color(0, get_channel_id(), 0, 0, color);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::set_brightness(char brightness) {
  supla_device *device = get_device();
  if (device) {
    device->set_brightness(0, get_channel_id(), 0, 0, brightness);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::set_color_brightness(char brightness) {
  supla_device *device = get_device();
  if (device) {
    device->set_color_brightness(0, get_channel_id(), 0, 0, brightness);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::action_toggle(void) {
  supla_device *device = get_device();
  if (device) {
    device->action_toggle(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::action_shut(
    const char *closingPercentage) {
  supla_device *device = get_device();
  if (device) {
    device->action_shut(0, get_channel_id(), 0, 0, closingPercentage);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::action_reveal(void) {
  supla_device *device = get_device();
  if (device) {
    device->action_reveal(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::action_stop(void) {
  supla_device *device = get_device();
  if (device) {
    device->action_stop(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::action_open(void) {
  supla_device *device = get_device();
  if (device) {
    device->action_open(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_mqtt_channel_value_setter::action_open_close(void) {
  supla_device *device = get_device();
  if (device) {
    device->action_open_close(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}
