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

#include "doubles/ipc/IpcSocketAdapterMock.h"

#include <string>

namespace testing {

IpcSocketAdapterMock::IpcSocketAdapterMock(int sfd)
    : supla_abstract_ipc_socket_adapter(sfd) {
  this->recv_buffer = NULL;
  this->recv_buffer_size = 0;
  this->recv_buffer_offset = 0;
}

IpcSocketAdapterMock::~IpcSocketAdapterMock() {}

bool IpcSocketAdapterMock::recv_byte(char *b) {
  if (recv_buffer && recv_buffer_size &&
      recv_buffer_offset < recv_buffer_size) {
    *b = recv_buffer[recv_buffer_offset];
    recv_buffer_offset++;
    return true;
  }
  return false;
}

void IpcSocketAdapterMock::set_recv_buffer(char *recv_buffer,
                                           unsigned int recv_buffer_size) {
  this->recv_buffer = recv_buffer;
  this->recv_buffer_size = recv_buffer_size;
  recv_buffer_offset = 0;
}

}  // namespace testing
