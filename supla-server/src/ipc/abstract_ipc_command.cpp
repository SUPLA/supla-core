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

#include "ipc/abstract_ipc_command.h"

#include <stdio.h>
#include <string.h>

#include "ipc/abstract_ipc_ctrl.h"
#include "log.h"

supla_abstract_ipc_command::supla_abstract_ipc_command(
    supla_abstract_ipc_socket_adapter *socket_adapter) {
  this->socket_adapter = socket_adapter;
}

supla_abstract_ipc_command::~supla_abstract_ipc_command() {}

template <typename T>
void supla_abstract_ipc_command::send_result(const char *result,
                                             const char *format, T value) {
  unsigned int size = strnlen(result, IPC_BUFFER_MAX_SIZE) + 20;
  char *buffer = (char *)malloc(size);
  if (buffer) {
    snprintf(buffer, size, format, result, value);
    socket_adapter->send(buffer);
    free(buffer);
  }
}

void supla_abstract_ipc_command::send_result(const char *result) {
  send_result<const char *>(result, "%s%s", "\n");
}

void supla_abstract_ipc_command::send_result(const char *result, int i) {
  send_result<int>(result, "%s%i\n", i);
}

void supla_abstract_ipc_command::send_result(const char *result, double d) {
  send_result<double>(result, "%s%f\n", d);
}

void supla_abstract_ipc_command::process_parameters(
    const char *params, const char *failure_suffix,
    std::function<bool(int user_id, int device_id, int channel_id)> on_ids) {
  int user_id = 0;
  int device_id = 0;
  int channel_id = 0;

  if (params) {
    sscanf(params, "%i,%i,%i", &user_id, &device_id, &channel_id);

    if (user_id && device_id && channel_id &&
        on_ids(user_id, device_id, channel_id)) {
      return;
    }
  }
  send_result(failure_suffix, channel_id);
}

void supla_abstract_ipc_command::process_parameters(
    const char *params,
    std::function<bool(int user_id, int device_id, int channel_id)> on_ids) {
  process_parameters(params, "UNKNOWN:", on_ids);
}

bool supla_abstract_ipc_command::process_command(char *buffer,
                                                 unsigned int buffer_size,
                                                 unsigned int data_size) {
  const char *cmd = get_command_name();

  unsigned int cmd_len = strnlen(cmd, buffer_size);
  if (data_size > cmd_len && memcmp(buffer, cmd, cmd_len) == 0 &&
      buffer[data_size - 1] == '\n') {
    buffer[data_size - 1] = 0;

    on_command_match(data_size > cmd_len ? &buffer[cmd_len] : NULL);
    return true;
  }

  return false;
}
