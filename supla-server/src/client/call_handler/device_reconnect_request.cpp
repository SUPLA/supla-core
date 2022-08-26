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

#include "client/call_handler/device_reconnect_request.h"

#include <string.h>

#include <memory>

#include "client/client.h"
#include "device/device.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_device_reconnect_request::supla_ch_device_reconnect_request(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_device_reconnect_request::~supla_ch_device_reconnect_request() {}

bool supla_ch_device_reconnect_request::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST) {
    return CH_UNHANDLED;
  }

  if (rd->data.cs_device_reconnect_request != nullptr) {
    TSC_DeviceReconnectRequestResult result = {};
    result.ResultCode = SUPLA_RESULTCODE_FALSE;

    if (client->is_superuser_authorized()) {
      shared_ptr<supla_device> device = client->get_user()->get_devices()->get(
          rd->data.cs_device_reconnect_request->DeviceID);
      if (device) {
        device->reconnect();
        result.ResultCode = SUPLA_RESULTCODE_TRUE;
      }

    } else {
      result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
    }

    srpc_adapter->sc_async_device_reconnect_request_result(&result);
  }

  return CH_HANDLED;
}
