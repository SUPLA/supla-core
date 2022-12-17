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
#include "user/user.h"

using std::shared_ptr;

supla_ch_get_channel_value_with_auth::supla_ch_get_channel_value_with_auth(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_get_channel_value_with_auth::~supla_ch_get_channel_value_with_auth() {}

bool supla_ch_get_channel_value_with_auth::can_handle_call(
    unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_GET_CHANNEL_VALUE_WITH_AUTH;
}

void supla_ch_get_channel_value_with_auth::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_get_value_with_auth == nullptr || client->is_registered()) {
    return;
  }

  TCS_ClientAuthorizationDetails* auth = &rd->data.cs_get_value_with_auth->Auth;
  int channel_id = rd->data.cs_get_value_with_auth->ChannelId;

  auth->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
  auth->AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE - 1] = 0;

  supla_db_access_provider dba;
  supla_client_dao client_dao(&dba);
  supla_connection_dao conn_dao(&dba);

  supla_register_client regcli;
  regcli.authenticate(client, auth, srpc_adapter, &dba, &conn_dao, &client_dao,
                      true, nullptr);

  TSC_GetChannelValueResult result = {};
  result.ChannelId = channel_id;

  if (regcli.get_result_code() != SUPLA_RESULTCODE_TRUE) {
    result.ResultCode = regcli.get_result_code();
  } else {
    result.ResultCode = SUPLA_RESULTCODE_CHANNEL_IS_OFFLINE;

    shared_ptr<supla_device> device =
        supla_user::get_device(regcli.get_user_id(), 0, channel_id);
    if (device != nullptr) {
      char online = 0;
      bool r = device->get_user()->get_channel_value(
          device->get_id(), channel_id, result.Value.value,
          result.Value.sub_value, &result.Value.sub_value_type,
          &result.ExtendedValue, &result.Function, &online, nullptr, true);

      if (r && online) {
        result.ResultCode = SUPLA_RESULTCODE_TRUE;
      }
    }

    srpc_adapter->sc_async_get_channel_value_result(&result);
  }
}

bool supla_ch_get_channel_value_with_auth::is_registration_required(void) {
  return false;
}
