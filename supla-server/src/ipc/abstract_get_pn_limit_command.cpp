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

#include "abstract_get_pn_limit_command.h"

#include <string.h>

#include "ipc/abstract_ipc_ctrl.h"

using std::string;

supla_abstract_get_pn_limit_command::supla_abstract_get_pn_limit_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const string supla_abstract_get_pn_limit_command::get_command_name(void) {
  return "PN-GET-LIMIT:";
}

void supla_abstract_get_pn_limit_command::on_command_match(const char *params) {
  int user_id = 0;
  unsigned int limit = 0;
  long left = 0;

  if (params) {
    sscanf(params, "%i", &user_id);

    if (user_id) {
      limit = get_pn_limit(user_id, &left);
    }
  }

  char *buffer = NULL;
  size_t size = 0;

  for (char a = 0; a < 2; a++) {
    int n = snprintf(buffer, size, "PN-LIMIT:%u,%ld", limit, left);

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

  if (buffer) {
    send_result(buffer);
    free(buffer);
  }
}
