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

#include "client/call_handler/register_client_b.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "log.h"
#include "proto.h"

using std::shared_ptr;

supla_ch_register_client_b::supla_ch_register_client_b(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_register_client_b::~supla_ch_register_client_b() {}

bool supla_ch_register_client_b::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_REGISTER_CLIENT_B) {
    return false;
  }

  supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT_B");

  rd->data.cs_register_client_b->AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE - 1] =
      0;
  rd->data.cs_register_client_b->Name[SUPLA_CLIENT_NAME_MAXSIZE - 1] = 0;
  rd->data.cs_register_client_b->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;
  rd->data.cs_register_client_b->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] = 0;

  // TODO(przemyslawzygmunt): Replace the old implementation with the new one
  // if (client->register_client(rd->data.cs_register_client_b, NULL,
  //                            proto_version) == 1) {
  //  set_registered(REG_CLIENT);
  //}

  return true;
}
