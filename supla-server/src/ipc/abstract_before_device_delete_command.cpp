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

#include <ipc/abstract_before_device_delete_command.h>

supla_abstract_before_device_delete_command::
    supla_abstract_before_device_delete_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const std::string supla_abstract_before_device_delete_command::get_command_name(
    void) {
  return "USER-BEFORE-DEVICE-DELETE:";
}

void supla_abstract_before_device_delete_command::on_command_match(
    const char *params) {
  int user_id = 0;
  int device_id = 0;

  if (params) {
    sscanf(params, "%i,%i", &user_id, &device_id);

    if (user_id && device_id) {
      before_device_delete(user_id, device_id);
      send_result("OK:", user_id);
      return;
    }
  }
  send_result("UNKNOWN:", user_id);
}
