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

#include "ipc/set_digiglass_value_command.h"

#include "device.h"
#include "http/httprequestqueue.h"
#include "user.h"

supla_set_digiglass_value_command::supla_set_digiglass_value_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_set_digiglass_value_command(socket_adapter) {}

bool supla_set_digiglass_value_command::set_channel_char_value(
    int user_id, int device_id, int channel_id, unsigned short active_bits,
    unsigned short mask) {
  bool result = false;
  supla_user::access_device(
      user_id, device_id, channel_id,
      [&result, channel_id, active_bits, mask,
       this](supla_device *device) -> void {
        result = device->get_channels()->set_dgf_transparency(
            get_caller(), channel_id, active_bits, mask);
      });

  return result;
}
