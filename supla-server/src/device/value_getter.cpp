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

#include "value_getter.h"

#include <memory>

#include "device/device.h"
#include "user/user.h"

using std::shared_ptr;

supla_channel_value *supla_value_getter::_get_value(int user_id, int device_id,
                                                    int channel_id, int *func,
                                                    bool *online) {
  shared_ptr<supla_device> device =
      supla_user::get_device(user_id, device_id, channel_id);

  supla_channel_value *result = nullptr;

  if (device != nullptr) {
    device->get_channels()->access_channel(
        channel_id,
        [&result, &func, &online](supla_device_channel *channel) -> void {
          result = channel->get_value<supla_channel_value>();
          if (func) {
            *func = channel->get_func();
          }
          if (online) {
            *online = !channel->is_offline();
          }
        });
  }

  return result;
}

supla_value_getter::~supla_value_getter(void) {}
