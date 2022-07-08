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

#include "ipc/abstract_client_reconnect_command.h"

supla_abstract_client_reconnect_command::
    supla_abstract_client_reconnect_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const std::string supla_abstract_client_reconnect_command::get_command_name(
    void) {
  return "CLIENT-RECONNECT:";
}

void supla_abstract_client_reconnect_command::on_command_match(
    const char *params) {
  if (params) {
    int user_id = 0;
    int client_id = 0;
    sscanf(params, "%i,%i", &user_id, &client_id);

    if (user_id && client_id && reconnect(user_id, client_id)) {
      send_result("OK:", client_id);
      return;
    }
  }

  send_result("USER_OR_CLIENT_UNKNOWN");
}
