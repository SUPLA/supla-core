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

#include "device/call_handler/get_channel_functions.h"

#include <memory>

#include "device/device_dao.h"

using std::map;
using std::shared_ptr;

supla_ch_get_channel_functions::supla_ch_get_channel_functions(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_get_channel_functions::~supla_ch_get_channel_functions() {}

bool supla_ch_get_channel_functions::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS) {
    return CH_UNHANDLED;
  }

  TSD_ChannelFunctions result = {};
  map<int, int> fncs = device->get_channels()->get_functions();

  for (auto it = fncs.cbegin(); it != fncs.cend(); ++it) {
    int channel_number = it->first;
    int channel_func = it->second;

    if (channel_number >= 0 && channel_number < SUPLA_CHANNELMAXCOUNT) {
      if (result.ChannelCount >= channel_number) {
        result.ChannelCount = channel_number + 1;
      }

      result.Functions[channel_number] = channel_func;
    }
  }

  srpc_adapter->sd_async_get_channel_functions_result(&result);

  return CH_HANDLED;
}
