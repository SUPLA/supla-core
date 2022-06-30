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

#ifndef SUPLA_ABSTRACT_IPC_CTRL_H_
#define SUPLA_ABSTRACT_IPC_CTRL_H_

#include <vector>

#define IPC_BUFFER_MAX_SIZE 4096
#define IPC_DEFAULT_TIMEOUT_SEC 5

#include "ipc/abstract_ipc_command.h"
#include "ipc/abstract_ipc_socket_adapter.h"

class supla_abstract_ipc_ctrl {
 private:
  char buffer[4096];
  bool timeout;
  unsigned char timeout_sec;
  supla_abstract_ipc_socket_adapter *socket_adapter;
  std::vector<supla_abstract_ipc_command *> commands;

 protected:
  virtual bool is_terminated(void) = 0;
  virtual void terminate(void) = 0;

 public:
  explicit supla_abstract_ipc_ctrl(
      supla_abstract_ipc_socket_adapter *socket_adapter);
  void execute(void);
  bool is_timeout(void);
  void set_timeout(unsigned char timeout);
  void add_command(supla_abstract_ipc_command *command);
  virtual ~supla_abstract_ipc_ctrl();
};

#endif /* SUPLA_ABSTRACT_IPC_CTRL_H_ */
