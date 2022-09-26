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

#include "client/call_handler/execute_action.h"

#include "actions/action_executor.h"
#include "client/client.h"
#include "client/clientchannelgroups.h"
#include "client/clientchannels.h"

using std::shared_ptr;

supla_ch_execute_action::supla_ch_execute_action(void)
    : supla_ch_abstract_execute_action() {}

supla_ch_execute_action::~supla_ch_execute_action() {}

bool supla_ch_execute_action::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_EXECUTE_ACTION;
}

void supla_ch_execute_action::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_action == nullptr) {
    return;
  }

  supla_action_executor aexec;
  execute_action(
      client->get_user_id(), client->get_id(), &aexec, rd->data.cs_action,
      srpc_adapter,
      [&client](int subject_type, int subject_id) -> bool {
        switch (subject_type) {
          case ACTION_SUBJECT_TYPE_CHANNEL:
            return client->get_channels()->channel_exists(subject_id);
          case ACTION_SUBJECT_TYPE_CHANNEL_GROUP:
            return client->get_cgroups()->groupExists(subject_id);
          case ACTION_SUBJECT_TYPE_SCENE:
            return client->get_scenes()->object_exists(subject_id);
        }
        return false;
      },
      [&client](int channel_id) -> bool {
        shared_ptr<supla_device> device =
            supla_user::get_device(client->get_user_id(), 0, channel_id);
        if (device != nullptr) {
          return device->get_channels()->is_channel_online(channel_id);
        }
        return false;
      });
}
