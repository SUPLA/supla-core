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

supla_abstract_ipc_command::supla_abstract_ipc_command(
    supla_abstract_ipc_response_agent *response_agent, char *buffer,
    unsigned int buffer_size) {
  this->response_agent = response_agent;
  this->buffer = buffer;
  this->buffer_size = buffer_size;
}

supla_abstract_ipc_command::~supla_abstract_ipc_command() {}

void supla_abstract_ipc_command::send_result(const char *result) {
  snprintf(buffer, buffer_size, "%s\n", result);
  response_agent->send_response(buffer);
}

void supla_abstract_ipc_command::send_result(const char *result, int i) {
  snprintf(buffer, buffer_size, "%s%i\n", result, i);
  response_agent->send_response(buffer);
}

void supla_abstract_ipc_command::send_result(const char *result, double i) {
  snprintf(buffer, buffer_size, "%s%f\n", result, i);
  response_agent->send_response(buffer);
}

bool supla_abstract_ipc_command::match_command(unsigned int data_size) {
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
