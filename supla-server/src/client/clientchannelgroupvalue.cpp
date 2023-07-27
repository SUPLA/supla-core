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

#include "clientchannelgroupvalue.h"

#include <string.h>

#include "client.h"
#include "clientchannelgroups.h"
#include "user.h"

supla_client_channelgroup_value::supla_client_channelgroup_value(
    supla_client_channelgroups *Container, int ChannelId, int DeviceId)
    : supla_client_objcontainer_item(Container, ChannelId, NULL) {
  this->DeviceId = DeviceId;
}

bool supla_client_channelgroup_value::remote_update_is_possible(void) {
  return true;
}

int supla_client_channelgroup_value::getChannelId(void) { return get_id(); }

int supla_client_channelgroup_value::getDeviceId(void) { return DeviceId; }

int supla_client_channelgroup_value::get_extra_id(void) { return DeviceId; }

void supla_client_channelgroup_value::proto_get(
    TSC_SuplaChannelValue *channel_value) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue));
  channel_value->Id = get_id();

  getContainer()->getClient()->get_user()->get_channel_value(
      DeviceId, get_id(), channel_value->value.value,
      channel_value->value.sub_value, nullptr, nullptr, nullptr,
      &channel_value->online, nullptr, true);
}

void supla_client_channelgroup_value::proto_get(
    TSC_SuplaChannelValue_B *channel_value) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue_B));
  channel_value->Id = get_id();

  getContainer()->getClient()->get_user()->get_channel_value(
      DeviceId, get_id(), channel_value->value.value,
      channel_value->value.sub_value, &channel_value->value.sub_value_type,
      nullptr, nullptr, &channel_value->online, nullptr, true);
}
