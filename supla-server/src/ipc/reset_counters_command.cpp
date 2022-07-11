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

#include "ipc/reset_counters_command.h"

#include "device.h"
#include "user.h"

supla_reset_counters_command::supla_reset_counters_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_reset_counters_command(socket_adapter) {}

bool supla_reset_counters_command::reset_counters(int user_id, int device_id,
                                                  int channel_id) {
  int result = false;

  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id](supla_device *device) -> void {
        result = device->get_channels()->reset_counters(channel_id);
      });

  return result;
}
