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

#include "client/call_handler/set_registration_enabled.h"

#include <string.h>

#include <memory>

#include "client/client.h"
#include "client/client_dao.h"
#include "db/db_access_provider.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_set_registration_enabled::supla_ch_set_registration_enabled(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_set_registration_enabled::~supla_ch_set_registration_enabled() {}

bool supla_ch_set_registration_enabled::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_SET_REGISTRATION_ENABLED;
}

void supla_ch_set_registration_enabled::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_set_registration_enabled != NULL) {
    TSC_SetRegistrationEnabledResult result = {};

    if (client->is_superuser_authorized()) {
      supla_db_access_provider dba;
      supla_client_dao dao(&dba);

      bool success = dao.set_reg_enabled(
          client->get_user_id(),
          rd->data.cs_set_registration_enabled->IODeviceRegistrationTimeSec,
          rd->data.cs_set_registration_enabled->ClientRegistrationTimeSec);

      result.ResultCode =
          success ? SUPLA_RESULTCODE_TRUE : SUPLA_RESULTCODE_UNKNOWN_ERROR;
    } else {
      result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
    }
    srpc_adapter->sc_async_set_registration_enabled_result(&result);
  }
}
