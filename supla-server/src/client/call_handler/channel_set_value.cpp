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

#include "client/call_handler/channel_set_value.h"

#include <string.h>

#include <memory>

#include "client.h"
#include "log.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"

using std::shared_ptr;

supla_ch_channel_set_value::supla_ch_channel_set_value(void)
    : supla_abstract_client_srpc_call_handler() {}

supla_ch_channel_set_value::~supla_ch_channel_set_value() {}

bool supla_ch_channel_set_value::handle_call(
    shared_ptr<supla_client> client, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (call_id != SUPLA_CS_CALL_CHANNEL_SET_VALUE) {
    return CH_UNHANDLED;
  }

  supla_log(LOG_DEBUG, "SUPLA_CS_CALL_CHANNEL_SET_VALUE");

  if (rd->data.cs_channel_new_value != nullptr) {
    TCS_SuplaChannelNewValue_B* cs_channel_new_value_b =
        (TCS_SuplaChannelNewValue_B*)malloc(sizeof(TCS_SuplaChannelNewValue_B));

    if (cs_channel_new_value_b != nullptr) {
      memset(cs_channel_new_value_b, 0, sizeof(TCS_SuplaChannelNewValue_B));
      cs_channel_new_value_b->ChannelId =
          rd->data.cs_channel_new_value->ChannelId;
      memcpy(cs_channel_new_value_b->value,
             rd->data.cs_channel_new_value->value, SUPLA_CHANNELVALUE_SIZE);
    }

    free(rd->data.cs_channel_new_value);
    rd->data.cs_channel_new_value_b = cs_channel_new_value_b;

    return client->get_srpc_call_handler_collection()->handle_call(
        client, srpc_adapter, rd, SUPLA_CS_CALL_CHANNEL_SET_VALUE_B,
        proto_version);
  }

  return CH_HANDLED;
}
