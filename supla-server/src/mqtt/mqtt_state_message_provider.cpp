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

#include <mqtt_state_message_provider.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "user.h"

using std::shared_ptr;

supla_mqtt_state_message_provider::supla_mqtt_state_message_provider(void)
    : supla_mqtt_abstract_state_message_provider() {}

supla_mqtt_state_message_provider::~supla_mqtt_state_message_provider(void) {}

const char *supla_mqtt_state_message_provider::_get_user_suid(void) {
  supla_user *user = supla_user::find(get_user_id(), false);
  if (user != NULL) {
    return user->getShortUniqueID();
  }

  return NULL;
}

channel_complex_value *supla_mqtt_state_message_provider::_get_complex_value(
    int user_id, int device_id, int channel_id) {
  channel_complex_value *result = NULL;
  supla_user *user = supla_user::find(get_user_id(), false);
  if (user != NULL) {
    channel_complex_value cvalue =
        user->get_channel_complex_value(get_channel_id());
    result = (channel_complex_value *)malloc(sizeof(channel_complex_value));
    memcpy(result, &cvalue, sizeof(channel_complex_value));
  }

  return result;
}

supla_channel_electricity_measurement *
supla_mqtt_state_message_provider::_get_electricity_measurement(void) {
  supla_channel_electricity_measurement *result = nullptr;
  shared_ptr<supla_device> device =
      supla_user::get_device(get_user_id(), get_device_id(), get_channel_id());
  if (device != nullptr) {
    device->get_channels()->access_channel(
        get_channel_id(), [&result](supla_device_channel *channel) -> void {
          result = channel->get_electricity_measurement(false);
        });
  }

  return result;
}

supla_channel_ic_measurement *
supla_mqtt_state_message_provider::_get_impulse_counter_measurement(void) {
  supla_channel_ic_measurement *result = nullptr;
  shared_ptr<supla_device> device =
      supla_user::get_device(get_user_id(), get_device_id(), get_channel_id());
  if (device != nullptr) {
    device->get_channels()->access_channel(
        get_channel_id(), [&result](supla_device_channel *channel) -> void {
          result = channel->get_impulse_counter_measurement(false);
        });
  }

  return result;
}
