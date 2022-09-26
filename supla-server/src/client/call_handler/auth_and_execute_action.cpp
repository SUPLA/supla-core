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

#include "client/call_handler/auth_and_execute_action.h"

#include "actions/action_executor.h"
#include "client/call_handler/register_client.h"
#include "client/client_dao.h"
#include "conn/authkey_cache.h"
#include "conn/connection_dao.h"

using std::shared_ptr;

supla_ch_auth_and_execute_action::supla_ch_auth_and_execute_action(void)
    : supla_ch_abstract_execute_action() {}

supla_ch_auth_and_execute_action::~supla_ch_auth_and_execute_action() {}

bool supla_ch_auth_and_execute_action::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_AUTH_AND_EXECUTE_ACTION;
}

void supla_ch_auth_and_execute_action::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_action_with_auth == nullptr || client->is_registered()) {
    return;
  }

  TCS_Action* action = &rd->data.cs_action_with_auth->Action;
  TCS_ClientAuthorizationDetails* auth = &rd->data.cs_action_with_auth->Auth;

  auth->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
  auth->AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE - 1] = 0;

  supla_db_access_provider dba;
  supla_client_dao client_dao(&dba);
  supla_connection_dao conn_dao(&dba);

  supla_register_client regcli;
  regcli.authenticate(client, auth, srpc_adapter, &dba, &conn_dao, &client_dao,
                      true);

  if (regcli.get_result_code() != SUPLA_RESULTCODE_TRUE) {
    send_result(action, srpc_adapter, regcli.get_result_code());
    return;
  }

  supla_user::find(regcli.get_user_id(), true);

  supla_action_executor aexec;
  execute_action(
      regcli.get_user_id(), regcli.get_client_id(), &aexec, action,
      srpc_adapter,
      [&client_dao, &regcli](int subject_type, int subject_id) -> bool {
        switch (subject_type) {
          case ACTION_SUBJECT_TYPE_CHANNEL:
            return client_dao.channel_exists(regcli.get_client_id(),
                                             subject_id);
          case ACTION_SUBJECT_TYPE_CHANNEL_GROUP:
            return client_dao.channel_group_exists(regcli.get_client_id(),
                                                   subject_id);
          case ACTION_SUBJECT_TYPE_SCENE:
            return client_dao.scene_exists(regcli.get_client_id(), subject_id);
        }
        return false;
      },
      [&regcli](int channel_id) -> bool {
        shared_ptr<supla_device> device =
            supla_user::get_device(regcli.get_user_id(), 0, channel_id);
        if (device != nullptr) {
          return device->get_channels()->is_channel_online(channel_id);
        }
        return false;
      });
}

bool supla_ch_auth_and_execute_action::is_registration_required(void) {
  return false;
}
