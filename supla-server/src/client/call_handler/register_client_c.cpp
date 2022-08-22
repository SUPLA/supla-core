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

#include "client/call_handler/register_client_c.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_register_client_c::supla_ch_register_client_c(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_register_client_c::~supla_ch_register_client_c() {}

bool supla_ch_register_client_c::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_REGISTER_CLIENT_C) {
    return false;
  }

  supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT_C");

  if (rd->data.cs_register_client_c != nullptr) {
    TCS_SuplaRegisterClient_D* register_client_d =
        (TCS_SuplaRegisterClient_D*)malloc(sizeof(TCS_SuplaRegisterClient_D));
    if (register_client_d != nullptr) {
      memset(register_client_d, 0, sizeof(TCS_SuplaRegisterClient_D));

      memcpy(register_client_d->Email, rd->data.cs_register_client_c->Email,
             SUPLA_EMAIL_MAXSIZE);
      memcpy(register_client_d->AuthKey, rd->data.cs_register_client_c->AuthKey,
             SUPLA_AUTHKEY_SIZE);
      memcpy(register_client_d->GUID, rd->data.cs_register_client_c->GUID,
             SUPLA_GUID_SIZE);
      memcpy(register_client_d->Name, rd->data.cs_register_client_c->Name,
             SUPLA_CLIENT_NAME_MAXSIZE);
      memcpy(register_client_d->SoftVer, rd->data.cs_register_client_c->SoftVer,
             SUPLA_SOFTVER_MAXSIZE);
      memcpy(register_client_d->ServerName,
             rd->data.cs_register_client_c->ServerName,
             SUPLA_SERVER_NAME_MAXSIZE);

      free(rd->data.cs_register_client_c);
      rd->data.cs_register_client_d = register_client_d;

      return client->get_srpc_call_handler_collection()->handle_call(
          client, srpc_adapter, rd, SUPLA_CS_CALL_REGISTER_CLIENT_D,
          proto_version);
    }
  }

  return true;
}
