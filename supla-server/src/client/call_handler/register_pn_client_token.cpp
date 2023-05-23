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

#include "client/client.h"
#include "client/client_dao.h"
#include "db/db_access_provider.h"
#include "log.h"

using std::shared_ptr;

supla_ch_register_pn_client_token::supla_ch_register_pn_client_token(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_register_pn_client_token::~supla_ch_register_pn_client_token() {}

bool supla_ch_register_pn_client_token::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_REGISTER_PN_CLIENT_TOKEN;
}

void supla_ch_register_pn_client_token::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_register_pn_client_token) {
    TCS_RegisterPnClientToken* reg = rd->data.cs_register_pn_client_token;
    if (reg->RealTokenSize > SUPLA_PN_CLIENT_TOKEN_MAXSIZE) {
      supla_log(
          LOG_WARNING,
          "The size of the push token is actually larger than what was passed "
          "to the server. Real Size: %i, Platform: %i, AppId: %i",
          reg->RealTokenSize, reg->Platform, reg->AppId);
    }
    if (reg->TokenSize <= SUPLA_PN_CLIENT_TOKEN_MAXSIZE) {
      supla_db_access_provider dba;
      supla_client_dao dao(&dba);
      reg->Token[reg->TokenSize > 1 ? reg->TokenSize - 1 : 0] = 0;
      dao.update_client_push_notification_client_token(
          client->get_user_id(), client->get_id(), (char*)reg->Token,
          reg->Platform, reg->AppId, reg->DevelopmentEnv);
    }
  }
}
