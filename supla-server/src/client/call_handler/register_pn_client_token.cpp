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

#include "register_pn_client_token.h"

#include <string.h>

#include <memory>

#include "client/call_handler/register_client.h"
#include "client/client.h"
#include "client/client_dao.h"
#include "conn/connection_dao.h"
#include "db/db_access_provider.h"
#include "log.h"

using std::shared_ptr;
using std::string;

supla_ch_register_pn_client_token::supla_ch_register_pn_client_token(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_register_pn_client_token::~supla_ch_register_pn_client_token() {}

bool supla_ch_register_pn_client_token::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_REGISTER_PN_CLIENT_TOKEN;
}

void supla_ch_register_pn_client_token::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  TSC_RegisterPnClientTokenResult result = {};
  result.ResultCode = SUPLA_RESULTCODE_FALSE;

  if (rd->data.cs_register_pn_client_token) {
    TCS_RegisterPnClientToken* reg = rd->data.cs_register_pn_client_token;
    supla_db_access_provider dba;
    supla_client_dao client_dao(&dba);

    int user_id = client->get_user_id();
    int client_id = client->get_id();

    if (!client->is_registered()) {
      string client_name;

      supla_connection_dao conn_dao(&dba);
      supla_register_client regcli;
      regcli.authenticate(client, &reg->Auth, srpc_adapter, &dba, &conn_dao,
                          &client_dao, true, &client_name);

      if (regcli.get_result_code() != SUPLA_RESULTCODE_TRUE) {
        result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
        srpc_adapter->sc_async_register_pn_client_token_result(&result);
        return;
      }

      user_id = regcli.get_user_id();
      client_id = regcli.get_client_id();
    }

    TCS_PnClientToken* token = &reg->Token;

    if (token->RealTokenSize > SUPLA_PN_CLIENT_TOKEN_MAXSIZE) {
      supla_log(
          LOG_WARNING,
          "The size of the push token is actually larger than what was passed "
          "to the server. Real Size: %i, Platform: %i, AppId: %i",
          token->RealTokenSize, token->Platform, token->AppId);

      result.ResultCode = SUPLA_RESULTCODE_INCORRECT_PARAMETERS;
    }
    if (reg->Token.TokenSize <= SUPLA_PN_CLIENT_TOKEN_MAXSIZE) {
      token->Token[token->TokenSize > 1 ? token->TokenSize - 1 : 0] = 0;
      client_dao.update_client_push_notification_client_token(
          user_id, client_id, (char*)token->Token, token->Platform,
          token->AppId, token->DevelopmentEnv);
      result.ResultCode = SUPLA_RESULTCODE_TRUE;
    }
  }

  srpc_adapter->sc_async_register_pn_client_token_result(&result);
}

bool supla_ch_register_pn_client_token::is_registration_required(void) {
  return false;
}
