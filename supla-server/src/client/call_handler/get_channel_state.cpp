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

#include "client/call_handler/get_channel_state.h"

#include <memory>

#include "client/client.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_get_channel_state::supla_ch_get_channel_state(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_get_channel_state::~supla_ch_get_channel_state() {}

bool supla_ch_get_channel_state::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CSD_CALL_GET_CHANNEL_STATE) {
    return CH_UNHANDLED;
  }

  if (rd->data.csd_channel_state_request != NULL) {
    TCSD_ChannelStateRequest* request = rd->data.csd_channel_state_request;
    if (request->ChannelID) {
      int device_id = client->get_channels()->get_device_id(request->ChannelID);
      if (device_id) {
        shared_ptr<supla_device> device =
            client->get_user()->get_devices()->get(device_id);
        if (device != nullptr) {
          return device->get_channels()->get_channel_state(
              supla_caller(ctClient, client->get_id()), request);
        }
      }
    }
  }

  return CH_HANDLED;
}
