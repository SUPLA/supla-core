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

#ifndef SUPLA_ABSTRACT_IPC_COMMAND_H_
#define SUPLA_ABSTRACT_IPC_COMMAND_H_

#include <ipc/abstract_ipc_response_agent.h>

class supla_abstract_ipc_command {
 private:
  supla_abstract_ipc_response_agent *response_agent;

  char *buffer;
  unsigned int buffer_size;

 protected:
  void send_result(const char *result);
  void send_result(const char *result, int i);
  void send_result(const char *result, double i);

  virtual void on_command_match(const char *params) = 0;
  virtual const char *get_command_name(void) = 0;

 public:
  supla_abstract_ipc_command(supla_abstract_ipc_response_agent *response_agent,
                             char *buffer, unsigned int buffer_size);
  virtual ~supla_abstract_ipc_command();
  bool match_command(unsigned int data_len);
};

#endif /* SUPLA_ABSTRACT_IPC_COMMAND_H_ */
