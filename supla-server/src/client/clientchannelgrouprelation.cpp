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

#include "clientchannelgrouprelation.h"
#include <string.h>

supla_client_channelgroup_relation::supla_client_channelgroup_relation(
    int ChannelId, int GroupId)
    : supla_client_objcontainer_item(ChannelId, NULL) {
  this->GroupId = GroupId;
}

supla_client_channelgroup_relation::supla_client_channelgroup_relation(
    supla_client_channelgroup_relation *cgr)
    : supla_client_objcontainer_item(cgr) {
  this->GroupId = cgr->GroupId;
}

bool supla_client_channelgroup_relation::remote_update_is_possible(void) {
  return true;
}

void supla_client_channelgroup_relation::update(
    supla_client_channelgroup_relation *cgr) {
  this->GroupId = cgr->GroupId;
}

int supla_client_channelgroup_relation::getExtraId() { return GroupId; }

int supla_client_channelgroup_relation::getChannelId(void) { return getId(); }

int supla_client_channelgroup_relation::getGroupId(void) { return GroupId; }
