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

#include "ipc/abstract_factory_reset_command.h"

#include <cstdio>
#include <string>

using std::string;

supla_abstract_factory_reset_command::supla_abstract_factory_reset_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_calcfg_ipc_command(socket_adapter) {}

const string supla_abstract_factory_reset_command::get_command_name(void) {
  return "FACTORY-RESET:";
}

void supla_abstract_factory_reset_command::on_command_match(
    const char *params) {
  int user_id = 0;
  int device_id = 0;
  unsigned _supla_int64_t queued_at = 0;
  bool waiting_for_result = false;

  if (params) {
    sscanf(params, "%i,%i", &user_id, &device_id);

    if (user_id && device_id &&
        factory_reset(user_id, device_id, &queued_at, &waiting_for_result)) {
      send_calcfg_result("OK:", device_id, device_id, queued_at,
                         waiting_for_result);
      return;
    }
  }
  send_result("UNKNOWN:", device_id);
}
