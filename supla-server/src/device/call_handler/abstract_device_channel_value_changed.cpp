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

#include "device/call_handler/abstract_device_channel_value_changed.h"

#include <stdlib.h>
#include <string.h>

#include <memory>

#include "device.h"
#include "log.h"
#include "proto.h"
#include "user.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;

supla_ch_abstract_device_channel_value_changed::
    supla_ch_abstract_device_channel_value_changed(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_abstract_device_channel_value_changed::
    ~supla_ch_abstract_device_channel_value_changed() {}

void supla_ch_abstract_device_channel_value_changed::on_channel_value_changed(
    shared_ptr<supla_device> device, unsigned char channel_number, char *value,
    unsigned char offline, unsigned _supla_int_t *validity_time_sec) {
  int channel_id = device->get_channels()->get_channel_id(channel_number);

  if (channel_id == 0) {
    return;
  }

  supla_channel_availability_status status;
  status.set_proto_offline(offline);

  if (status.is_offline()) {
    device->get_channels()->set_channel_availability_status(channel_id, status);
  } else {
    device->get_channels()->set_channel_value(channel_id, value,
                                              validity_time_sec, &status);
  }
}
