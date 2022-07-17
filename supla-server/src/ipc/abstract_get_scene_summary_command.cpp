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

#include "ipc/abstract_get_scene_summary_command.h"

#include <string.h>

#include "ipc/abstract_ipc_ctrl.h"
#include "tools.h"

supla_abstract_get_scene_summary_command::
    supla_abstract_get_scene_summary_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const std::string supla_abstract_get_scene_summary_command::get_command_name(
    void) {
  return "GET-SCENE-SUMMARY:";
}

void supla_abstract_get_scene_summary_command::on_command_match(
    const char *params) {
  int user_id = 0;
  int scene_id = 0;

  if (params) {
    sscanf(params, "%i,%i", &user_id, &scene_id);

    supla_scene_state state;
    if (user_id && scene_id && get_scene_state(user_id, scene_id, &state)) {
      char *initiator_name_b64 =
          st_openssl_base64_encode(state.get_caller().get_name().c_str(),
                                   state.get_caller().get_name().size());

      char *buffer = NULL;
      size_t size = 0;

      for (char a = 0; a < 2; a++) {
        int n = snprintf(
            buffer, size, "SUMMARY:%i,%i,%i,%s,%u,%u", scene_id,
            state.get_caller().get_type(), state.get_caller().get_id(),
            initiator_name_b64 ? initiator_name_b64 : "",
            state.get_milliseconds_from_start(), state.get_milliseconds_left());

        if (a == 0) {
          if (n > -1) { /* glibc 2.1 */
            size = n + 1;
          } else { /* glibc 2.0 */
            size = IPC_BUFFER_MAX_SIZE;
          }

          if (size) {
            buffer = (char *)malloc(size);
            memset(buffer, 0, size);
          }
        }
      }

      if (initiator_name_b64) {
        free(initiator_name_b64);
        initiator_name_b64 = NULL;
      }

      if (buffer) {
        send_result(buffer);
        free(buffer);
        return;
      }
    }
  }
  send_result("UNKNOWN:", scene_id);
}
