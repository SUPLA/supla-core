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

#include "device/call_handler/device_channel_value_changed.h"

#include <memory>

#include "proto.h"

using std::shared_ptr;

supla_ch_device_channel_value_changed::supla_ch_device_channel_value_changed(
    void)
    : supla_ch_abstract_device_channel_value_changed() {}

supla_ch_device_channel_value_changed::
    ~supla_ch_device_channel_value_changed() {}

bool supla_ch_device_channel_value_changed::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED) {
    return false;
  }

  if (rd->data.ds_device_channel_value != nullptr) {
    on_channel_value_changed(
        device, rd->data.ds_device_channel_value->ChannelNumber,
        rd->data.ds_device_channel_value->value, false, nullptr);
  }

  return true;
}
