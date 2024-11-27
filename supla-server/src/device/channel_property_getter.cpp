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

#include "device/channel_property_getter.h"

#include <memory>

#include "device/device.h"
#include "user/user.h"

using std::shared_ptr;

supla_channel_property_getter::~supla_channel_property_getter(void) {}

supla_channel_value *supla_channel_property_getter::_get_value(
    int user_id, int device_id, int channel_id,
    supla_channel_fragment *fragment,
    supla_channel_availability_status *status) {
  supla_channel_value *result = nullptr;

  supla_user *user = supla_user::get_user(user_id);
  if (!user) {
    return result;
  }

  shared_ptr<supla_device> device =
      user->get_devices()->get(device_id, channel_id);

  if (fragment) {
    *fragment = user->get_devices()->get_channel_fragment(channel_id);
  }

  if (device == nullptr) {
    if (status) {
      status->set_offline(true);
    }
  } else {
    device->get_channels()->access_channel(
        channel_id, [&result, &status](supla_device_channel *channel) -> void {
          result = channel->get_value<supla_channel_value>();
          if (status) {
            *status = channel->get_availability_status();
          }
        });
  }

  return result;
}

supla_channel_extended_value *
supla_channel_property_getter::_get_extended_value(int user_id, int device_id,
                                                   int channel_id) {
  supla_user *user = supla_user::get_user(user_id);

  if (!user) {
    return nullptr;
  }

  supla_channel_extended_value *result = nullptr;

  shared_ptr<supla_device> device =
      user->get_devices()->get(device_id, channel_id);

  if (device) {
    device->get_channels()->access_channel(
        channel_id, [&result](supla_device_channel *channel) -> void {
          result =
              channel->get_extended_value<supla_channel_extended_value>(false);
        });
  }

  return result;
}

int supla_channel_property_getter::_get_func(int user_id, int device_id,
                                             int channel_id) {
  int result = 0;
  supla_user *user = supla_user::get_user(user_id);

  if (!user) {
    return result;
  }

  shared_ptr<supla_device> device =
      user->get_devices()->get(device_id, channel_id);

  if (device == nullptr) {
    supla_channel_fragment f =
        user->get_devices()->get_channel_fragment(channel_id);
    result = f.get_function();
  } else {
    device->get_channels()->access_channel(
        channel_id, [&result](supla_device_channel *channel) -> void {
          result = channel->get_func();
        });
  }

  return result;
}

supla_json_config *supla_channel_property_getter::_get_detached_json_config(
    int user_id, int device_id, int channel_id) {
  shared_ptr<supla_device> device =
      supla_user::get_device(user_id, device_id, channel_id);

  supla_json_config *result = nullptr;

  if (device != nullptr) {
    device->get_channels()->access_channel(
        channel_id, [&result](supla_device_channel *channel) -> void {
          result = channel->get_json_config();
        });
  }

  return result;
}

int supla_channel_property_getter::_get_channel_id(
    int user_id, int device_id, unsigned char channel_number) {
  shared_ptr<supla_device> device =
      supla_user::get_device(user_id, device_id, 0);

  int result = 0;

  if (device != nullptr) {
    result = device->get_channels()->get_channel_id(channel_number);
  }

  return result;
}
