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

#include "ipc/abstract_ipc_ctrl.h"

#include <string.h>
#include <sys/time.h>

#include "log.h"

using std::string;
using std::vector;

supla_abstract_ipc_ctrl::supla_abstract_ipc_ctrl(
    supla_abstract_ipc_socket_adapter *socket_adapter) {
  this->socket_adapter = socket_adapter;
  this->timeout = false;
  this->timeout_sec = IPC_DEFAULT_TIMEOUT_SEC;
  memset(buffer, 0, sizeof(buffer));
}

supla_abstract_ipc_ctrl::~supla_abstract_ipc_ctrl() {
  for (auto it = commands.begin(); it != commands.end(); ++it) {
    delete *it;
  }
  delete socket_adapter;
}

void supla_abstract_ipc_ctrl::add_command(supla_abstract_ipc_command *command) {
  commands.push_back(command);
}

void supla_abstract_ipc_ctrl::execute(void) {
  if (socket_adapter == NULL) {
    return;
  }

  char buffer[IPC_BUFFER_MAX_SIZE] = {};

  struct timeval last_action = {};
  struct timeval now = {};

  gettimeofday(&last_action, NULL);

  socket_adapter->send_data("SUPLA SERVER CTRL\n");

  unsigned int offset = 0;

  while (!is_terminated()) {
    char c = 0;
    socket_adapter->wait(1000000);
    while (socket_adapter->recv_byte(&c) && !socket_adapter->is_error() &&
           !is_terminated()) {
      buffer[offset] = c;
      offset++;
      if (offset >= sizeof(buffer)) {
        offset = 0;
      }

      if (c == '\n') {
        bool result = false;

        for (auto it = commands.begin(); it != commands.end(); ++it) {
          if ((*it)->process_command(buffer, sizeof(buffer), offset)) {
            result = true;
            break;
          }
        }

        if (result) {
          gettimeofday(&last_action, NULL);
        } else {
          supla_log(LOG_WARNING, "IPC - COMMAND UNKNOWN: %s", buffer);
          socket_adapter->send_data("COMMAND_UNKNOWN\n");
          terminate();
        }
        offset = 0;
      }

      if (offset == 0) {
        memset(buffer, 0, sizeof(buffer));
      }
    }

    if (socket_adapter->is_error()) {
      terminate();
      break;
    }

    gettimeofday(&now, NULL);

    if (now.tv_sec - last_action.tv_sec >= timeout_sec) {
      terminate();
      timeout = true;
      break;
    }
  }
}

bool supla_abstract_ipc_ctrl::is_timeout(void) { return timeout; }

void supla_abstract_ipc_ctrl::set_timeout(unsigned char timeout_sec) {
  this->timeout_sec = timeout_sec;
}

vector<string> supla_abstract_ipc_ctrl::get_command_list(void) {
  vector<string> result;

  for (auto it = commands.begin(); it != commands.end(); ++it) {
    result.push_back((*it)->get_command_name());
  }

  return result;
}
