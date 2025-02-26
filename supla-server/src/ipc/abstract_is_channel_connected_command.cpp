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

#include "ipc/abstract_is_channel_connected_command.h"

using std::string;

supla_abstract_is_channel_connected_command::
    supla_abstract_is_channel_connected_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const string supla_abstract_is_channel_connected_command::get_command_name(
    void) {
  return "IS-CHANNEL-CONNECTED:";
}

void supla_abstract_is_channel_connected_command::on_command_match(
    const char *params) {
  process_parameters(
      params, "DISCONNECTED:",
      [this](int user_id, int device_id, int channel_id) -> bool {
        supla_channel_availability_status status =
            get_availability_status(user_id, device_id, channel_id);

        if (status.is_online()) {
          send_result("CONNECTED:", channel_id);
          return true;
        } else if (status.is_online_but_not_available()) {
          send_result("CONNECTED_BUT_NOT_AVAILABLE:", channel_id);
          return true;
        } else if (status.is_offline_remote_wakeup_not_supported()) {
          send_result("OFFLINE_REMOTE_WAKEUP_NOT_SUPPORTED:", channel_id);
          return true;
        } else if (status.is_firmware_update_ongoing()) {
          send_result("FIRMWARE_UPDATE_ONGOING:", channel_id);
          return true;
        }

        return false;
      });
}
