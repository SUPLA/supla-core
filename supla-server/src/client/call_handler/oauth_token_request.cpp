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

#include "client/call_handler/oauth_token_request.h"

#include <string.h>

#include <memory>

#include "client/client.h"
#include "client/client_dao.h"
#include "db/db_access_provider.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_oauth_token_request::supla_ch_oauth_token_request(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_oauth_token_request::~supla_ch_oauth_token_request() {}

bool supla_ch_oauth_token_request::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST;
}

void supla_ch_oauth_token_request::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  TSC_OAuthTokenRequestResult result = {};

  int access_id = client->getAccessID();

  result.ResultCode = SUPLA_OAUTH_RESULTCODE_ERROR;

  if (access_id) {
    supla_db_access_provider dba;
    supla_client_dao dao(&dba);

    bool storage_connect_error = false;

    if (dao.oauth_get_token(&result.Token, client->get_user()->getUserID(),
                            access_id, &storage_connect_error)) {
      result.ResultCode = SUPLA_OAUTH_RESULTCODE_SUCCESS;
    } else {
      memset(&result.Token, 0, sizeof(TSC_OAuthToken));
      if (storage_connect_error) {
        result.ResultCode = SUPLA_OAUTH_TEMPORARILY_UNAVAILABLE;
      }
    }
  }

  srpc_adapter->cs_async_oauth_token_request_result(&result);
}
