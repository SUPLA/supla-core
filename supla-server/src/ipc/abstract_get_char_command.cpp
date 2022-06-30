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

#include "ipc/abstract_get_char_command.h"

supla_abstract_get_char_command::supla_abstract_get_char_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter),
      command_name("GET-CHAR-VALUE:") {}

const char *supla_abstract_get_char_command::get_command_name(void) {
  return command_name.c_str();
}

void supla_abstract_get_char_command::on_command_match(const char *params) {
  process_parameters(
      params, [this](int user_id, int device_id, int channel_id) -> bool {
        char value = 0;

        if (get_channel_char_value(user_id, device_id, channel_id, &value)) {
          send_result("VALUE:", (int)value);
          return true;
        }

        return false;
      });
}
