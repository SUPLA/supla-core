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

#include "client/call_handler/register_client.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_register_client::supla_ch_register_client(void)
    : supla_abstract_srpc_call_handler() {}

supla_ch_register_client::~supla_ch_register_client() {}

bool supla_ch_register_client::handle_call(
    std::shared_ptr<supla_abstract_connection_object> object,
    supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
    unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_REGISTER_CLIENT) {
    return false;
  }

  supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT");

  if (rd->data.cs_register_client != NULL) {
    TCS_SuplaRegisterClient_B* register_client_b =
        (TCS_SuplaRegisterClient_B*)malloc(sizeof(TCS_SuplaRegisterClient_B));

    if (register_client_b != NULL) {
      memset(register_client_b, 0, sizeof(TCS_SuplaRegisterClient_B));

      register_client_b->AccessID = rd->data.cs_register_client->AccessID;
      memcpy(register_client_b->AccessIDpwd,
             rd->data.cs_register_client->AccessIDpwd,
             SUPLA_ACCESSID_PWD_MAXSIZE);
      memcpy(register_client_b->GUID, rd->data.cs_register_client->GUID,
             SUPLA_GUID_SIZE);
      memcpy(register_client_b->Name, rd->data.cs_register_client->Name,
             SUPLA_CLIENT_NAME_MAXSIZE);
      memcpy(register_client_b->SoftVer, rd->data.cs_register_client->SoftVer,
             SUPLA_SOFTVER_MAXSIZE);
    }

    free(rd->data.cs_register_client);
    rd->data.cs_register_client_b = register_client_b;

    return object->get_srpc_call_handler_collection()->handle_call(
        object, srpc_adapter, rd, SUPLA_CS_CALL_REGISTER_CLIENT_B,
        proto_version);
  }

  return true;
}
