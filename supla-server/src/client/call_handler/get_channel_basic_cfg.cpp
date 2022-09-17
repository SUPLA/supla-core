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

#include "client/call_handler/get_channel_basic_cfg.h"

#include <memory>

#include "client/client.h"

using std::shared_ptr;

supla_ch_get_channel_basic_cfg::supla_ch_get_channel_basic_cfg(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_get_channel_basic_cfg::~supla_ch_get_channel_basic_cfg() {}

bool supla_ch_get_channel_basic_cfg::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG;
}

void supla_ch_get_channel_basic_cfg::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.cs_channel_basic_cfg_request != NULL) {
    TSC_ChannelBasicCfg basic_cfg = {};

    client->get_channels()->channel_access(
        rd->data.cs_channel_basic_cfg_request->ChannelID,
        [&basic_cfg](supla_client_channel* channel) -> void {
          if (!channel->get_basic_cfg(&basic_cfg)) {
            basic_cfg.ID = 0;
          }
        });

    if (basic_cfg.ID) {
      srpc_adapter->sc_async_channel_basic_cfg_result(&basic_cfg);
    }
  }
}
