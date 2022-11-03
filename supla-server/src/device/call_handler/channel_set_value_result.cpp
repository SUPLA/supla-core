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

#include "device/call_handler/channel_set_value_result.h"

#include <string.h>

#include <memory>

#include "client.h"
#include "device.h"
#include "user.h"

using std::shared_ptr;

supla_ch_channel_set_value_result::supla_ch_channel_set_value_result(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_channel_set_value_result::~supla_ch_channel_set_value_result() {}

bool supla_ch_channel_set_value_result::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT;
}

void supla_ch_channel_set_value_result::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.ds_channel_new_value_result == nullptr) {
    return;
  }

  TDS_SuplaChannelNewValueResult* result = rd->data.ds_channel_new_value_result;

  if (result == nullptr || result->SenderID == 0) {
    return;
  }

  int channel_id =
      device->get_channels()->get_channel_id(result->ChannelNumber);

  if (channel_id == 0) {
    return;
  }

  int channel_type = device->get_channels()->get_channel_type(channel_id);

  if (result->Success == 1 || channel_type == SUPLA_CHANNELTYPE_BRIDGE) {
    TSC_SuplaEvent event = {};
    event.ChannelID = channel_id;
    event.SenderID = result->SenderID;
    event.DurationMS =
        device->get_channels()->get_channel_value_duration(channel_id);

    std::shared_ptr<supla_client> client =
        device->get_user()->get_clients()->get(result->SenderID);
    if (client != nullptr) {
      client->getName(event.SenderName, SUPLA_SENDER_NAME_MAXSIZE);
    }

    event.SenderNameSize =
        strnlen(event.SenderName, SUPLA_SENDER_NAME_MAXSIZE - 1) + 1;

    if (result->Success == 0 && channel_type == SUPLA_CHANNELTYPE_BRIDGE) {
      event.Event = SUPLA_EVENT_SET_BRIDGE_VALUE_FAILED;
    } else {
      switch (device->get_channels()->get_channel_func(channel_id)) {
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEGATEWAYLOCK;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEGATE;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEGARAGEDOOR;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEDOORLOCK;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEROLLERSHUTTER;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEROOFWINDOW;
          break;
        case SUPLA_CHANNELFNC_POWERSWITCH:
          event.Event = SUPLA_EVENT_POWERONOFF;
          break;
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
          event.Event = SUPLA_EVENT_LIGHTONOFF;
          break;
        case SUPLA_CHANNELFNC_STAIRCASETIMER:
          event.Event = SUPLA_EVENT_STAIRCASETIMERONOFF;
          break;
        case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
          event.Event = SUPLA_EVENT_VALVEOPENCLOSE;
          break;
      }
    }

    device->get_user()->get_clients()->call_event(&event);
  }
}
