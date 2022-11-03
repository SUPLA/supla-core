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

#include "device/call_handler/device_channel_extendedvalue_changed.h"

#include <memory>

#include "device.h"
#include "proto.h"
#include "user.h"

using std::shared_ptr;

supla_ch_device_channel_extendedvalue_changed::
    supla_ch_device_channel_extendedvalue_changed(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_device_channel_extendedvalue_changed::
    ~supla_ch_device_channel_extendedvalue_changed() {}

bool supla_ch_device_channel_extendedvalue_changed::can_handle_call(
    unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED;
}

void supla_ch_device_channel_extendedvalue_changed::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.ds_device_channel_extendedvalue != nullptr) {
    int channel_id = device->get_channels()->get_channel_id(
        rd->data.ds_device_channel_extendedvalue->ChannelNumber);

    if (channel_id != 0) {
      device->get_channels()->set_channel_extendedvalue(
          channel_id, &rd->data.ds_device_channel_extendedvalue->value);

      device->get_user()->on_channel_value_changed(
          supla_caller(ctDevice, device->get_id()), device->get_id(),
          channel_id, true);
    }
  }
}
