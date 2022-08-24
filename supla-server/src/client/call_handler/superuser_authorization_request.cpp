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

#include "client/call_handler/superuser_authorization_request.h"

#include <memory>

#include "client/client.h"

using std::shared_ptr;

supla_ch_superuser_authorization_request::
    supla_ch_superuser_authorization_request(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_superuser_authorization_request::
    ~supla_ch_superuser_authorization_request() {}

bool supla_ch_superuser_authorization_request::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST) {
    return CH_UNHANDLED;
  }

  if (rd->data.cs_superuser_authorization_request != NULL) {
    rd->data.cs_superuser_authorization_request
        ->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
    rd->data.cs_superuser_authorization_request
        ->Password[SUPLA_PASSWORD_MAXSIZE - 1] = 0;

    bool connection_failed = false;

    client->superuser_authorize(
        client->get_user_id(),
        rd->data.cs_superuser_authorization_request->Email,
        rd->data.cs_superuser_authorization_request->Password,
        &connection_failed);

    TSC_SuperUserAuthorizationResult result = {};
    if (client->is_superuser_authorized()) {
      result.Result = SUPLA_RESULTCODE_AUTHORIZED;
    } else if (connection_failed) {
      result.Result = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
    } else {
      result.Result = SUPLA_RESULTCODE_UNAUTHORIZED;
    }

    srpc_adapter->sc_async_superuser_authorization_result(&result);
  }

  return CH_HANDLED;
}
