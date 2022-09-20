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

#include "client/call_handler/calcfg_request_b.h"

#include <memory>

#include "client.h"
#include "user.h"
#include "userchannelgroups.h"

using std::shared_ptr;

supla_ch_calcfg_request_b::supla_ch_calcfg_request_b(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_calcfg_request_b::~supla_ch_calcfg_request_b() {}

bool supla_ch_calcfg_request_b::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B;
}

void supla_ch_calcfg_request_b::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  TCS_DeviceCalCfgRequest_B* request = rd->data.cs_device_calcfg_request_b;

  if (request != NULL) {
    if (request->Target == SUPLA_TARGET_CHANNEL ||
        request->Target == SUPLA_TARGET_IODEVICE) {
      int device_id = 0;
      int channel_id = 0;

      if (request->Target == SUPLA_TARGET_CHANNEL) {
        channel_id = request->Id;
        device_id = client->get_channels()->get_device_id(channel_id);
      } else {
        device_id = request->Id;
        channel_id =
            client->get_channels()->get_any_channel_id_with_deviceid(device_id);
      }

      if (device_id && channel_id) {
        client->get_user()->device_calcfg_request(
            supla_caller(ctClient, client->get_id()), device_id, channel_id,
            request);
      }

    } else if (request->Target == SUPLA_TARGET_GROUP) {
      if (client->get_cgroups()->groupExists(
              request->Id)) {  // Make sure the client has access to this group
        client->get_user()->get_channel_groups()->calcfg_request(
            supla_caller(ctClient, client->get_id()), request);
      }
    }
  }
}
