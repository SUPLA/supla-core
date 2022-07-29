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

#ifndef SUPLA_ABSTRACT_IPC_SOCKET_ADAPTER_H_
#define SUPLA_ABSTRACT_IPC_SOCKET_ADAPTER_H_

#include <string>

#include "eh.h"

class supla_abstract_ipc_socket_adapter {
 private:
  int sfd;
  TEventHandler *eh;

 public:
  explicit supla_abstract_ipc_socket_adapter(int sfd);
  virtual ~supla_abstract_ipc_socket_adapter();
  void wait(int usec);
  int get_sfd(void);
  virtual void send_data(const std::string &data) = 0;
  virtual bool recv_byte(char *b) = 0;
  virtual bool is_error(void) = 0;
};

#endif /* SUPLA_ABSTRACT_IPC_SOCKET_ADAPTER_H_ */
