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

#include "device/call_handler/set_channel_caption.h"

#include <memory>

#include "device/device.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_set_channel_caption::supla_ch_set_channel_caption(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_set_channel_caption::~supla_ch_set_channel_caption() {}

bool supla_ch_set_channel_caption::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DCS_CALL_SET_CHANNEL_CAPTION;
}

void supla_ch_set_channel_caption::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.dcs_set_caption != nullptr) {
    int channel_id = device->get_channels()->get_channel_id(
        rd->data.dcs_set_caption->ChannelNumber);

    device->get_user()->set_caption(
        supla_caller(ctDevice, device->get_id()), true, channel_id,
        device->get_connection()->get_srpc_adapter()->get_srpc(),
        rd->data.dcs_set_caption, call_id, true);
  }
}
