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

#include "device/call_handler/channel_state_result.h"

#include <memory>

#include "client.h"
#include "device.h"
#include "user.h"

using std::shared_ptr;

supla_ch_channel_state_result::supla_ch_channel_state_result(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_channel_state_result::~supla_ch_channel_state_result() {}

bool supla_ch_channel_state_result::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DSC_CALL_CHANNEL_STATE_RESULT;
}

void supla_ch_channel_state_result::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.dsc_channel_state != nullptr) {
    int channel_id = device->get_channels()->get_channel_id(
        rd->data.dsc_channel_state->ChannelNumber);
    if (channel_id != 0) {
      shared_ptr<supla_client> client = device->get_user()->get_clients()->get(
          rd->data.dsc_channel_state->ReceiverID);
      if (client != nullptr) {
        client->on_device_channel_state_result(channel_id,
                                               rd->data.dsc_channel_state);
      }
    }
  }
}
