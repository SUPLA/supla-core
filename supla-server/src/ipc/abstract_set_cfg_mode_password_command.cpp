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

#include "abstract_set_cfg_mode_password_command.h"

#include <string.h>

#include "proto.h"
#include "tools.h"

using std::string;

supla_abstract_set_cfg_mode_password_command::
    supla_abstract_set_cfg_mode_password_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const string supla_abstract_set_cfg_mode_password_command::get_command_name(
    void) {
  return "SET-CFG-MODE-PASSWORD:";
}

void supla_abstract_set_cfg_mode_password_command::on_command_match(
    const char *params) {
  int user_id = 0;
  int device_id = 0;
  char password_base64[100] =
      {};  // 4 * ((SUPLA_PASSWORD_MAXSIZE + 2) / 3) == 88

  bool result = false;
  char *password = nullptr;

  if (params) {
    sscanf(params, "%i,%i,%100s", &user_id, &device_id, password_base64);
    password_base64[sizeof(password_base64) - 1] = 0;
    int len = 0;

    if (user_id && device_id) {
      password = st_openssl_base64_decode(
          password_base64, strnlen(password_base64, sizeof(password_base64)),
          &len);

      if (password && len && len < SUPLA_PASSWORD_MAXSIZE &&
          set_cfg_mode_password(user_id, device_id, password)) {
        result = true;
      }
    }
  }

  if (password) {
    free(password);
  }

  send_result(result ? "OK:" : "UNKNOWN:", device_id);
}
