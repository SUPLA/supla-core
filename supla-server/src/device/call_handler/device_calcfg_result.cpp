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

#include "device/call_handler/device_calcfg_result.h"

#include <memory>

#include "client.h"
#include "device.h"
#include "user.h"

using std::shared_ptr;

supla_ch_device_calcfg_result::supla_ch_device_calcfg_result(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_device_calcfg_result::~supla_ch_device_calcfg_result() {}

bool supla_ch_device_calcfg_result::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DS_CALL_DEVICE_CALCFG_RESULT) {
    return false;
  }

  int channel_id = device->get_channels()->get_channel_id(
      rd->data.ds_device_calcfg_result->ChannelNumber);
  if (channel_id == 0) {
    return true;
  }

  if (rd->data.ds_device_calcfg_result->DataSize >=
          (sizeof(TCalCfg_ZWave_Node) - ZWAVE_NODE_NAME_MAXSIZE) &&
      rd->data.ds_device_calcfg_result->DataSize <=
          sizeof(TCalCfg_ZWave_Node)) {
    switch (rd->data.ds_device_calcfg_result->Command) {
      case SUPLA_CALCFG_CMD_ZWAVE_ADD_NODE:
      case SUPLA_CALCFG_CMD_ZWAVE_GET_NODE_LIST:
        TCalCfg_ZWave_Node* node =
            (TCalCfg_ZWave_Node*)rd->data.ds_device_calcfg_result->Data;
        if (node->Flags & ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED) {
          node->ChannelID =
              device->get_channels()->get_channel_id(node->ChannelNumber);
          if (node->ChannelID == 0) {
            node->Flags ^= ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED;
          }
        } else {
          node->ChannelID = 0;
        }
        break;
    }
  }

  shared_ptr<supla_client> client = device->get_user()->get_clients()->get(
      rd->data.ds_device_calcfg_result->ReceiverID);
  if (client != nullptr) {
    client->on_device_calcfg_result(channel_id,
                                    rd->data.ds_device_calcfg_result);
  }

  return true;
}
