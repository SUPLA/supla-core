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

#ifndef SUPLA_IPC_SOCKET_ADAPTER_H_
#define SUPLA_IPC_SOCKET_ADAPTER_H_

#include "eh.h"
#include "ipc/abstract_ipc_socket_adapter.h"

class supla_ipc_socket_adapter : public supla_abstract_ipc_socket_adapter {
 private:
  bool error;

 public:
  explicit supla_ipc_socket_adapter(int sfd);
  virtual ~supla_ipc_socket_adapter();
  virtual void send_data(const std::string &data);
  virtual bool recv_byte(char *b);
  virtual bool is_error(void);
};

#endif /* SUPLA_ABSTRACT_IPC_SOCKET_ADAPTER_H_ */
