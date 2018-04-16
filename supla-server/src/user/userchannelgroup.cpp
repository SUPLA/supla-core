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

#include "userchannelgroup.h"

supla_user_channelgroup::supla_user_channelgroup(
    supla_user_channelgroups *Container, int GroupId, int ChannelId,
    int DeviceId)
    : supla_objcontainer_item(Container, GroupId) {
  this->ChannelId = ChannelId;
  this->DeviceId = DeviceId;
}

int supla_user_channelgroup::getGroupId() { return getId(); }
int supla_user_channelgroup::getChannelId() { return ChannelId; }
int supla_user_channelgroup::getDeviceId() { return DeviceId; }
int supla_user_channelgroup::getExtraId() { return ChannelId; }
