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

#include "client/call_handler/register_client_d.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "client/client_dao.h"
#include "conn/connection_dao.h"
#include "log.h"
#include "proto.h"

using std::shared_ptr;

supla_ch_register_client_d::supla_ch_register_client_d(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_register_client_d::~supla_ch_register_client_d() {}

bool supla_ch_register_client_d::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_REGISTER_CLIENT_D) {
    return CH_UNHANDLED;
  }

  supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT_D");

  if (rd->data.cs_register_client_d != nullptr) {
    rd->data.cs_register_client_d->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
    rd->data.cs_register_client_d->Password[SUPLA_PASSWORD_MAXSIZE - 1] = 0;
    rd->data.cs_register_client_d->Name[SUPLA_CLIENT_NAME_MAXSIZE - 1] = 0;
    rd->data.cs_register_client_d->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;
    rd->data.cs_register_client_d->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] =
        0;

    supla_db_access_provider dba;
    supla_connection_dao conn_dao(&dba);
    supla_client_dao client_dao(&dba);

    register_client(client, nullptr, rd->data.cs_register_client_d,
                    srpc_adapter, &dba, &conn_dao, &client_dao,
                    client->get_connection()->get_client_sd(),
                    client->get_connection()->get_client_ipv4(),
                    client->get_connection()->get_activity_timeout());
  }

  return CH_HANDLED;
}
