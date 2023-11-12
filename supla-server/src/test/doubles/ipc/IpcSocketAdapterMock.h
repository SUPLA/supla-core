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

#ifndef IPC_SOCKET_ADAPTER_MOCK_H_
#define IPC_SOCKET_ADAPTER_MOCK_H_

#include <gmock/gmock.h>
#include <ipc/abstract_ipc_socket_adapter.h>

#include <string>

namespace testing {

class IpcSocketAdapterMock : public supla_abstract_ipc_socket_adapter {
 private:
  char *recv_buffer;
  unsigned int recv_buffer_size;
  unsigned int recv_buffer_offset;

 public:
  explicit IpcSocketAdapterMock(int sfd);
  virtual ~IpcSocketAdapterMock();

  MOCK_METHOD1(send_data, void(const std::string &response));
  MOCK_METHOD0(is_error, bool());
  virtual bool recv_byte(char *b);
  void set_recv_buffer(char *recv_buffer, unsigned int recv_buffer_size);
};

} /* namespace testing */

#endif /* IPC_SOCKET_ADAPTER_MOCK_H_ */
