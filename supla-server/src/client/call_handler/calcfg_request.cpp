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

#include "client/call_handler/calcfg_request.h"

#include <string.h>

#include <memory>

#include "proto.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_calcfg_request::supla_ch_calcfg_request(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_calcfg_request::~supla_ch_calcfg_request() {}

bool supla_ch_calcfg_request::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST) {
    return CH_UNHANDLED;
  }

  if (rd->data.cs_device_calcfg_request != NULL) {
    TCS_DeviceCalCfgRequest_B* cs_device_calcfg_request_b =
        (TCS_DeviceCalCfgRequest_B*)malloc(sizeof(TCS_DeviceCalCfgRequest_B));

    memset(cs_device_calcfg_request_b, 0, sizeof(TCS_DeviceCalCfgRequest_B));

    cs_device_calcfg_request_b->Id =
        rd->data.cs_device_calcfg_request->ChannelID;
    cs_device_calcfg_request_b->Target = SUPLA_TARGET_CHANNEL;
    cs_device_calcfg_request_b->Command =
        rd->data.cs_device_calcfg_request->Command;
    cs_device_calcfg_request_b->DataType =
        rd->data.cs_device_calcfg_request->DataType;
    cs_device_calcfg_request_b->DataSize =
        rd->data.cs_device_calcfg_request->DataSize;
    memcpy(cs_device_calcfg_request_b->Data,
           rd->data.cs_device_calcfg_request->Data, SUPLA_CALCFG_DATA_MAXSIZE);

    free(rd->data.cs_device_calcfg_request);
    rd->data.cs_device_calcfg_request = NULL;
    rd->data.cs_device_calcfg_request_b = cs_device_calcfg_request_b;

    return client->get_srpc_call_handler_collection()->handle_call(
        client, srpc_adapter, rd, SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B,
        proto_version);
  }

  return CH_HANDLED;
}
