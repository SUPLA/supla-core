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

#include "client/call_handler/get_channel_value_with_auth.h"

#include <memory>

#include "client/call_handler/register_client.h"
#include "client/client.h"
#include "client/client_dao.h"
#include "conn/connection_dao.h"
#include "device/device.h"
#include "device/extended_value/abstract_channel_extended_value.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_get_channel_value_with_auth::supla_ch_get_channel_value_with_auth(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_get_channel_value_with_auth::~supla_ch_get_channel_value_with_auth() {}

bool supla_ch_get_channel_value_with_auth::can_handle_call(
    unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_GET_CHANNEL_VALUE_WITH_AUTH;
}

supla_ch_get_channel_value_with_auth::authentication_result
supla_ch_get_channel_value_with_auth::authenticate(
    shared_ptr<supla_client> client, TCS_ClientAuthorizationDetails* auth,
    supla_abstract_srpc_adapter* srpc_adapter,
    supla_mariadb_access_provider* dba, supla_connection_dao* conn_dao,
    supla_client_dao* client_dao) {
  supla_register_client regcli;
  regcli.authenticate(client, auth, srpc_adapter, dba, conn_dao, client_dao,
                      true, nullptr);

  return {regcli.get_result_code(), regcli.get_client_id(),
          regcli.get_user_id()};
}

bool supla_ch_get_channel_value_with_auth::channel_exists(
    supla_client_dao* client_dao, int client_id, int channel_id) {
  return client_dao->channel_exists(client_id, channel_id);
}

void supla_ch_get_channel_value_with_auth::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_get_value_with_auth == nullptr || client->is_registered()) {
    return;
  }

  handle_request(client, srpc_adapter, rd->data.cs_get_value_with_auth);
}

void supla_ch_get_channel_value_with_auth::handle_request(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TCS_GetChannelValueWithAuth* request) {
  TCS_ClientAuthorizationDetails* auth = &request->Auth;
  int channel_id = request->ChannelId;

  auth->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
  auth->AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE - 1] = 0;

  supla_mariadb_access_provider dba;
  supla_client_dao client_dao(&dba);
  supla_connection_dao conn_dao(&dba);

  authentication_result auth_result =
      authenticate(client, auth, srpc_adapter, &dba, &conn_dao, &client_dao);

  TSC_GetChannelValueResult result = {};
  result.ChannelId = channel_id;

  if (auth_result.result_code != SUPLA_RESULTCODE_TRUE) {
    result.ResultCode = auth_result.result_code;
  } else if (channel_id == 0 ||
             !channel_exists(&client_dao, auth_result.client_id, channel_id)) {
    // The channel must be available to the authenticated client (its
    // AccessID), not just belong to the same user.
    result.ResultCode = SUPLA_RESULTCODE_SUBJECT_NOT_FOUND;
  } else {
    result.ResultCode = SUPLA_RESULTCODE_CHANNEL_IS_OFFLINE;

    supla_user* user = supla_user::find(auth_result.user_id, false);
    if (user) {
      supla_channel_availability_status status(true);

      supla_abstract_channel_extended_value* ev = nullptr;
      bool r = user->get_channel_value(
          0, channel_id, result.Value.value, result.Value.sub_value,
          &result.Value.sub_value_type, &ev, &result.Function, &status, nullptr,
          true);

      if (ev) {
        ev->get_raw_value(&result.ExtendedValue);
        delete ev;
      }

      if (r && status.is_online()) {
        result.ResultCode = SUPLA_RESULTCODE_TRUE;
      }
    }
  }

  // Always respond, also on authentication failure or when the channel is
  // not available to the client, so the caller does not wait for a timeout.
  srpc_adapter->sc_async_get_channel_value_result(&result);
}

bool supla_ch_get_channel_value_with_auth::is_registration_required(void) {
  return false;
}
