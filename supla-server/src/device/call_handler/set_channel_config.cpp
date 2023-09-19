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

#include "device/call_handler/set_channel_config.h"

#include <memory>

#include "client/client.h"
#include "device/device.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_set_channel_config::supla_ch_set_channel_config(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_set_channel_config::~supla_ch_set_channel_config() {}

bool supla_ch_set_channel_config::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_SET_CHANNEL_CONFIG;
}

void supla_ch_set_channel_config::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.sds_set_channel_config_request == nullptr) {
    return;
  }

  TSDS_SetChannelConfig* request = rd->data.sds_set_channel_config_request;

  TSDS_SetChannelConfigResult result = {};
  result.Result = SUPLA_CONFIG_RESULT_FALSE;
  result.ConfigType = request->ConfigType;
  result.ChannelNumber = request->ChannelNumber;

  int channel_id =
      device->get_channels()->get_channel_id(request->ChannelNumber);

  channel_json_config* json_config = nullptr;

  device->get_channels()->access_channel(
      channel_id, [&](supla_device_channel* channel) -> void {
        result.Result = channel->set_user_config(
            request->ConfigType, request->ConfigSize, request->Config);

        if (result.Result == SUPLA_CONFIG_RESULT_TRUE) {
          json_config = channel->get_json_config();
        }
      });

  device->get_connection()
      ->get_srpc_adapter()
      ->sd_async_set_channel_config_result(&result);

  device->get_channels()->access_channel(
      channel_id, [&](supla_device_channel* channel) -> void {
        channel->send_config_to_device(request->ConfigType);
      });

  if (result.Result == SUPLA_CONFIG_RESULT_TRUE) {
    device->get_user()->get_clients()->for_each(
        [&](std::shared_ptr<supla_client> client, bool* will_continue) -> void {
          client->get_channels()->channel_access(
              channel_id, [&](supla_client_channel* channel) -> void {
                channel->set_json_config(
                    json_config ? new channel_json_config(json_config, true)
                                : nullptr);

                TSC_ChannelConfigUpdateOrResult cfg_result = {};
                cfg_result.Result = SUPLA_CONFIG_RESULT_TRUE;
                channel->get_config(&cfg_result.Config, request->ConfigType,
                                    nullptr, 0);

                client->get_connection()
                    ->get_srpc_adapter()
                    ->sc_async_channel_config_update_or_result(&cfg_result);
              });
        });
  }

  if (json_config) {
    delete json_config;
  }
}
