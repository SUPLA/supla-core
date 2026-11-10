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

#include "client/call_handler/set_channel_config.h"

#include <memory>

#include "client/client.h"
#include "client/clientchannels.h"
#include "device/device.h"
#include "user/user.h"

using std::shared_ptr;

supla_client_ch_set_channel_config::supla_client_ch_set_channel_config(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_client_ch_set_channel_config::~supla_client_ch_set_channel_config() {}

bool supla_client_ch_set_channel_config::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_SET_CHANNEL_CONFIG;
}

void supla_client_ch_set_channel_config::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  supla_json_config* json_config = nullptr;
  TSCS_ChannelConfig* config = nullptr;
  TSC_ChannelConfigUpdateOrResult cfg_result = {};

  cfg_result.Result = SUPLA_CONFIG_RESULT_FALSE;

  // !If you add the option to change the configuration of a type other than
  // SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, remember to convert channel
  // identifiers/numbers!

  unsigned char real_config_type = SUPLA_CONFIG_TYPE_DEFAULT;

  if (rd->data.scs_channel_config != nullptr) {
    config = rd->data.scs_channel_config;
    real_config_type = config->ConfigType;

    if (config->ChannelId) {
      client->get_channels()->channel_access(
          config->ChannelId, [&](supla_client_channel* channel) -> void {
            if (config->ConfigType == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE) {
              cfg_result.Result = channel->set_user_config(
                  config->ConfigType, config->ConfigSize, config->Config);
            } else {
              cfg_result.Result = SUPLA_CONFIG_RESULT_NOT_ALLOWED;
            }

            channel->get_config(&cfg_result.Config, config->ConfigType,
                                &real_config_type, 0);

            if (cfg_result.Result == SUPLA_CONFIG_RESULT_TRUE) {
              json_config = channel->get_json_config();
            }
          });
    }
  }

  if (cfg_result.Result == SUPLA_CONFIG_RESULT_TRUE) {
    int client_id = client->get_id();

    client->get_user()->get_clients()->for_each(
        [&](std::shared_ptr<supla_client> client, bool* will_continue) -> void {
          if (client->get_id() == client_id) {
            return;
          }

          client->get_channels()->channel_access(
              config->ChannelId, [&](supla_client_channel* channel) -> void {
                channel->set_json_config(
                    json_config ? new supla_json_config(json_config, true)
                                : nullptr);

                client->get_connection()
                    ->get_srpc_adapter()
                    ->sc_async_channel_config_update_or_result(&cfg_result);
              });
        });

    // Send a new config to the device after sending the configuration to
    // clients because the device can correct the configuration.

    std::shared_ptr<supla_device> device =
        client->get_user()->get_devices()->get(0, config->ChannelId);

    if (device) {
      device->get_channels()->access_channel(
          config->ChannelId, [&](supla_device_channel* channel) -> void {
            channel->set_json_config(
                json_config ? new supla_json_config(json_config, true)
                            : nullptr);
            channel->send_config_to_device(real_config_type);
          });
    }
  }

  if (json_config) {
    delete json_config;
  }

  srpc_adapter->sc_async_channel_config_update_or_result(&cfg_result);
}
