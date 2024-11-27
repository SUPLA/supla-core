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

#include "update_channel_state_command.h"

#include <memory>

#include "device.h"
#include "user.h"

using std::shared_ptr;

supla_update_channel_state_command::supla_update_channel_state_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_update_channel_state_command(socket_adapter) {}

bool supla_update_channel_state_command::update_state(int user_id,
                                                      int device_id,
                                                      int channel_id) {
  shared_ptr<supla_device> device =
      supla_user::get_device(user_id, device_id, channel_id);
  if (device != nullptr) {
    TCSD_ChannelStateRequest request = {};
    request.ChannelID = channel_id;
    return device->get_channels()->get_channel_state_async(supla_caller(ctIPC),
                                                           &request);
  }
  return false;
}
