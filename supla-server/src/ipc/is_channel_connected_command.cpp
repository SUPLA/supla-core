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

#include "ipc/is_channel_connected_command.h"

#include <memory>

#include "user.h"

using std::shared_ptr;

supla_is_channel_connected_command::supla_is_channel_connected_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_is_channel_connected_command(socket_adapter) {}

bool supla_is_channel_connected_command::is_channel_online(int user_id,
                                                           int device_id,
                                                           int channel_id) {
  shared_ptr<supla_device> device =
      supla_user::get_device(user_id, device_id, channel_id);
  if (device != nullptr) {
    return device->get_channels()
        ->get_channel_availability_status(channel_id)
        .is_online();
  }
  return false;
}
