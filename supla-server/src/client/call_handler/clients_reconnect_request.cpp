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

#include "client/call_handler/clients_reconnect_request.h"

#include <string.h>

#include <memory>

#include "client/client.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_clients_reconnect_request::supla_ch_clients_reconnect_request(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_clients_reconnect_request::~supla_ch_clients_reconnect_request() {}

bool supla_ch_clients_reconnect_request::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST;
}

void supla_ch_clients_reconnect_request::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (client->is_superuser_authorized()) {
    client->get_user()->reconnect(supla_caller(ctClient, client->get_id()),
                                  false, true);
  } else {
    TSC_ClientsReconnectRequestResult result = {};
    result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
    srpc_adapter->sc_async_clients_reconnect_request_result(&result);
  }
}
