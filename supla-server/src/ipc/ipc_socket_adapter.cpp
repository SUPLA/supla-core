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

#include "ipc/ipc_socket_adapter.h"

#include <sys/socket.h>

supla_ipc_socket_adapter::supla_ipc_socket_adapter(int sfd)
    : supla_abstract_ipc_socket_adapter(sfd) {
  error = false;
}

supla_ipc_socket_adapter::~supla_ipc_socket_adapter() {}

void supla_ipc_socket_adapter::send_data(const std::string &data) {
  send(get_sfd(), data.c_str(), data.size(), 0);
}

bool supla_ipc_socket_adapter::recv_byte(char *b) {
  ssize_t r = recv(get_sfd(), b, 1, 0);
  if (r == 0) {
    error = true;
  }
  return r == 1;
}

bool supla_ipc_socket_adapter::is_error(void) { return error; }
