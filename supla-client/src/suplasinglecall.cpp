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

#include "suplasinglecall.h"

#include <string.h>

#include "proto.h"
#include "supla-socket.h"

using std::function;

#define RESPONSE_TIMEOUT_USEC 5000000

supla_single_call::supla_single_call(
    TCS_ClientAuthorizationDetails *auth_details, int protocol_version) {
  this->auth_details = *auth_details;
  this->protocol_version = protocol_version;
  ssd = nullptr;
  srpc = nullptr;
  eh = nullptr;
  min_version_required = 0;
}

supla_single_call::~supla_single_call() { cleanup(); }

// static
int supla_single_call::socket_read(void *buf, int count, void *single_call) {
  return static_cast<supla_single_call *>(single_call)->socket_read(buf, count);
}

// static
int supla_single_call::socket_write(void *buf, int count, void *single_call) {
  return static_cast<supla_single_call *>(single_call)
      ->socket_write(buf, count);
}

// static
void supla_single_call::on_remote_call_received(void *_srpc, unsigned int rr_id,
                                                unsigned int call_id,
                                                void *single_call,
                                                unsigned char proto_version) {
  static_cast<supla_single_call *>(single_call)
      ->on_remote_call_received(call_id);
}

// static
void supla_single_call::on_min_version_required(void *_srpc,
                                                unsigned _supla_int_t call_id,
                                                unsigned char min_version,
                                                void *single_call) {
  static_cast<supla_single_call *>(single_call)
      ->on_min_version_required(min_version);
}

int supla_single_call::socket_read(void *buf, int count) {
  return ssocket_read(ssd, nullptr, buf, count);
}

int supla_single_call::socket_write(void *buf, int count) {
  return ssocket_write(ssd, nullptr, buf, count);
}

void supla_single_call::on_remote_call_received(unsigned int call_id) {
  TsrpcReceivedData rd;
  if (SUPLA_RESULT_TRUE == srpc_getdata(srpc, &rd, 0)) {
    received_data.push_back(rd);
    eh_raise_event(eh);
  }
}

void supla_single_call::on_min_version_required(unsigned char min_version) {
  min_version_required = min_version;
  eh_raise_event(eh);
}

int supla_single_call::connect(void) {
  int err = SUPLA_RESULT_FALSE;

  ssd = ssocket_client_init(auth_details.ServerName, 2016, 1);

  if (ssocket_client_connect(ssd, nullptr, &err)) {
    eh = eh_init();
    eh_add_fd(eh, ssocket_get_fd(ssd));

    TsrpcParams srpc_params;
    srpc_params_init(&srpc_params);
    srpc_params.user_params = this;
    srpc_params.data_read = socket_read;
    srpc_params.data_write = socket_write;
    srpc_params.on_remote_call_received = on_remote_call_received;
    srpc_params.on_min_version_required = on_min_version_required;
    srpc_params.eh = eh;

    srpc = srpc_init(&srpc_params);

    if (protocol_version > 0) {
      srpc_set_proto_version(srpc, protocol_version);
    }

    return SUPLA_RESULT_TRUE;
  }

  return err;
}

void supla_single_call::cleanup(void) {
  for (auto it = received_data.begin(); it != received_data.end(); ++it) {
    srpc_rd_free(&(*it));
  }

  received_data.clear();

  if (ssd) {
    ssocket_free(ssd);
    ssd = nullptr;
  }

  if (eh) {
    eh_free(eh);
    eh = nullptr;
  }

  if (srpc) {
    srpc_free(srpc);
    srpc = nullptr;
  }
}

int supla_single_call::make_call(
    function<void(void)> call,
    function<bool(int *result, TsrpcReceivedData *rd)> process_response) {
  int result = SUPLA_RESULT_FALSE;
  min_version_required = 0;

  if ((result = connect()) != SUPLA_RESULT_TRUE) {
    cleanup();
    return result;
  }

  call();

  if (min_version_required) {
    result = SUPLA_RESULT_VERSION_ERROR;
    cleanup();
    return result;
  }

  struct timeval now = {};
  struct timeval then = {};
  gettimeofday(&then, NULL);

  result = SUPLA_RESULT_RESPONSE_TIMEOUT;
  bool waiting_for_response = true;

  while (srpc_iterate(srpc) != SUPLA_RESULT_FALSE && waiting_for_response) {
    eh_wait(eh, 5000000);

    for (auto it = received_data.begin(); it != received_data.end(); ++it) {
      if (process_response(&result, &(*it))) {
        waiting_for_response = false;
      }

      srpc_rd_free(&(*it));

      if (!waiting_for_response) {
        break;
      }
    }
    received_data.clear();

    gettimeofday(&now, NULL);
    unsigned long long time_usec = (now.tv_sec * 1000000 + now.tv_usec) -
                                   (then.tv_sec * 1000000 + then.tv_usec);
    if (time_usec >= RESPONSE_TIMEOUT_USEC) {
      break;
    }
  };

  cleanup();
  return result;
}

unsigned char supla_single_call::get_min_version_number(void) {
  return min_version_required;
}

int supla_single_call::execute_action(TCS_Action *action) {
  TCS_ActionWithAuth awa = {};
  memcpy(&awa.Action, action, sizeof(TCS_Action));
  memcpy(&awa.Auth, &auth_details, sizeof(TCS_ClientAuthorizationDetails));

  function<void(void)> call = [this, &awa]() -> void {
    srpc_cs_async_execute_action_with_auth(srpc, &awa);
  };

  function<bool(int *result, TsrpcReceivedData *rd)> process_response =
      [action](int *result, TsrpcReceivedData *rd) -> bool {
    if (rd->call_id == SUPLA_SC_CALL_ACTION_EXECUTION_RESULT &&
        rd->data.sc_action_execution_result) {
      TSC_ActionExecutionResult *aer = rd->data.sc_action_execution_result;
      if (aer->ActionId == action->ActionId &&
          aer->SubjectType == action->SubjectType &&
          aer->SubjectId == action->SubjectId) {
        *result = aer->ResultCode;
        return true;
      }
    }

    return false;
  };

  return make_call(call, process_response);
}
