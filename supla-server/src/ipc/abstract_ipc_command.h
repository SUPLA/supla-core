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

#include <functional>
#include <string>

#include "caller.h"
#include "ipc/abstract_ipc_socket_adapter.h"

class supla_abstract_ipc_ctrl;
class supla_abstract_ipc_command {
 private:
  supla_abstract_ipc_socket_adapter *socket_adapter;
  char *alexa_correlation_token;
  char *google_request_id;

  template <typename T>
  void send_result(const char *result, const char *format, T value);
  char *cut(const char *params, const char *var, unsigned int buffer_size);
  void free_correlation_token();
  void free_google_requestid();

 protected:
  void send_result(const char *result);
  void send_result(const char *result, int i);
  void send_result(const char *result, double d);

  void process_parameters(
      const char *params, const char *failure_suffix,
      std::function<bool(int user_id, int device_id, int channel_id)> on_ids);

  void process_parameters(
      const char *params,
      std::function<bool(int user_id, int device_id, int channel_id)> on_ids);

  virtual void on_command_match(const char *params) = 0;
  virtual const std::string get_command_name(void) = 0;
  const char *get_alexa_correlation_token();
  const char *get_google_request_id();
  supla_caller get_caller();

 public:
  explicit supla_abstract_ipc_command(
      supla_abstract_ipc_socket_adapter *socket_adapter);
  virtual ~supla_abstract_ipc_command();
  bool process_command(char *buffer, unsigned int buffer_size,
                       unsigned int data_size);
};

#endif /* SUPLA_ABSTRACT_IPC_COMMAND_H_ */
