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

#include "ipc/abstract_ipc_ctrl.h"
#include "log.h"
#include "tools.h"

using std::function;
using std::string;

supla_abstract_ipc_command::supla_abstract_ipc_command(
    supla_abstract_ipc_socket_adapter *socket_adapter) {
  this->socket_adapter = socket_adapter;
  this->alexa_correlation_token = NULL;
  this->google_request_id = NULL;
}

supla_abstract_ipc_command::~supla_abstract_ipc_command() {
  free_correlation_token();
  free_google_requestid();
}

template <typename T>
void supla_abstract_ipc_command::send_result(const char *result,
                                             const char *format, T value) {
  unsigned int size = strnlen(result, IPC_BUFFER_MAX_SIZE) + 20;
  char *buffer = (char *)malloc(size);
  if (buffer) {
    snprintf(buffer, size, format, result, value);
    socket_adapter->send_data(buffer);
    free(buffer);
  }
}

void supla_abstract_ipc_command::send_result(const char *result) {
  send_result<const char *>(result, "%s%s", "\n");
}

void supla_abstract_ipc_command::send_result(const char *result, int i) {
  send_result<int>(result, "%s%i\n", i);
}

void supla_abstract_ipc_command::send_result(const char *result, double d) {
  send_result<double>(result, "%s%f\n", d);
}

void supla_abstract_ipc_command::free_correlation_token() {
  if (alexa_correlation_token) {
    free(alexa_correlation_token);
    alexa_correlation_token = NULL;
  }
}

void supla_abstract_ipc_command::free_google_requestid() {
  if (google_request_id) {
    free(google_request_id);
    google_request_id = NULL;
  }
}

char *supla_abstract_ipc_command::base64_decode_cut(const char *params,
                                                    const char *needle,
                                                    unsigned int buffer_size) {
  char *result = NULL;

  char *ct = strstr((char *)params, needle);
  unsigned int var_len = strnlen(needle, 255);

  if (ct != NULL && strnlen(ct, buffer_size) > var_len) {
    char *value = &ct[var_len];

    int len = strnlen(value, buffer_size);

    if (len > 0) {
      result = st_openssl_base64_decode(value, len, NULL);
      if (strnlen(result, buffer_size) <= 0) {
        free(result);
        result = NULL;
      }
    }

    ct[0] = 0;
  }

  return result;
}

const char *supla_abstract_ipc_command::get_alexa_correlation_token() {
  return alexa_correlation_token;
}

const char *supla_abstract_ipc_command::get_google_request_id() {
  return google_request_id;
}

void supla_abstract_ipc_command::process_parameters(
    const char *params, const char *failure_suffix,
    function<bool(int user_id, int device_id, int channel_id)> on_ids) {
  int user_id = 0;
  int device_id = 0;
  int channel_id = 0;

  if (params) {
    sscanf(params, "%i,%i,%i", &user_id, &device_id, &channel_id);

    if (user_id && device_id && channel_id &&
        on_ids(user_id, device_id, channel_id)) {
      return;
    }
  }
  send_result(failure_suffix, channel_id);
}

void supla_abstract_ipc_command::process_parameters(
    const char *params,
    function<bool(int user_id, int device_id, int channel_id)> on_ids) {
  process_parameters(params, "UNKNOWN:", on_ids);
}

bool supla_abstract_ipc_command::process_command(char *buffer,
                                                 unsigned int buffer_size,
                                                 unsigned int data_size) {
  const string cmd = get_command_name();

  if (cmd.empty()) {
    return false;
  }

  if (data_size > cmd.size() && memcmp(buffer, cmd.c_str(), cmd.size()) == 0 &&
      buffer[data_size - 1] == '\n') {
    buffer[data_size - 1] = 0;

    free_correlation_token();
    free_google_requestid();

    alexa_correlation_token =
        base64_decode_cut(buffer, ",ALEXA-CORRELATION-TOKEN=", buffer_size);
    google_request_id =
        base64_decode_cut(buffer, ",GOOGLE-REQUEST-ID=", buffer_size);

    on_command_match(data_size > cmd.size() ? &buffer[cmd.size()] : NULL);
    return true;
  }

  return false;
}

supla_caller supla_abstract_ipc_command::get_caller() {
  supla_caller caller(alexa_correlation_token
                          ? ctAmazonAlexa
                          : (google_request_id ? ctGoogleHome : ctIPC));
  return caller;
}
