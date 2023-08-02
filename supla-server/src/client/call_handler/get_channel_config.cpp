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

#include "client/call_handler/get_channel_config.h"

#include <memory>

#include "client/client.h"
#include "client/clientchannels.h"
#include "user/user.h"

using std::shared_ptr;

supla_client_ch_get_channel_config::supla_client_ch_get_channel_config(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_client_ch_get_channel_config::~supla_client_ch_get_channel_config() {}

bool supla_client_ch_get_channel_config::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_GET_CHANNEL_CONFIG;
}

void supla_client_ch_get_channel_config::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_get_channel_config_request != nullptr) {
    TCS_GetChannelConfigRequest* request =
        rd->data.cs_get_channel_config_request;
    if (request->ChannelId) {
      TSC_ChannelConfigUpdate config = {};
      config.Result = SUPLA_CONFIG_RESULT_FALSE;

      client->get_channels()->channel_access(
          request->ChannelId, [&](supla_client_channel* channel) -> void {
            if (channel->get_config(&config.Config, request->ConfigType,
                                    request->Flags)) {
              config.Result = SUPLA_CONFIG_RESULT_TRUE;
            }
          });

      srpc_adapter->sc_async_channel_config_update(&config);
    }
  }
}
