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

#include "clientchannelgroup.h"
#include <string.h>
#include "client.h"
#include "log.h"
#include "proto.h"
#include "safearray.h"

supla_client_channelgroup::supla_client_channelgroup(
    supla_client_channelgroups *Container, int Id, int LocationID, int Func,
    const char *Caption, int AltIcon, int UserIcon)
    : supla_client_objcontainer_item(Container, Id, Caption) {
  this->LocationID = LocationID;
  this->Func = Func;
  this->AltIcon = AltIcon;
  this->UserIcon = UserIcon;
  this->Flags = 0;
  this->relarr = safe_array_init();
}

supla_client_channelgroup::~supla_client_channelgroup(void) {
  safe_array_free(this->relarr);
}

bool supla_client_channelgroup::add_relation(
    supla_client_channelgroup_relation *cg_rel) {
  if (cg_rel->getGroupId() != getId()) {
    return false;
  }

  bool result = true;
  supla_client_channelgroup_relation *item = NULL;

  safe_array_lock(relarr);
  for (int a = 0; a < safe_array_count(relarr); a++) {
    item = static_cast<supla_client_channelgroup_relation *>(
        safe_array_get(relarr, a));
    if (item && item->getChannelId() == cg_rel->getChannelId()) {
      result = false;
      break;
    }
  }

  if (result) {
    safe_array_add(relarr, cg_rel);
  }

  safe_array_unlock(relarr);

  return result;
}

bool supla_client_channelgroup::remote_update_is_possible(void) {
  if (getContainer()->getClient()->getProtocolVersion() < 9) {
    return false;
  }

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:

      return true;
  }

  return false;
}

void supla_client_channelgroup::proto_get(TSC_SuplaChannelGroup *group) {
  memset(group, 0, sizeof(TSC_SuplaChannelGroup));

  group->Id = getId();
  group->LocationID = LocationID;
  group->Func = Func;
  group->AltIcon = AltIcon;
  group->Flags = Flags;

  proto_get_caption(group->Caption, &group->CaptionSize,
                    SUPLA_CHANNELGROUP_CAPTION_MAXSIZE);
}

void supla_client_channelgroup::proto_get(TSC_SuplaChannelGroup_B *group) {
  memset(group, 0, sizeof(TSC_SuplaChannelGroup_B));

  group->Id = getId();
  group->LocationID = LocationID;
  group->Func = Func;
  group->AltIcon = AltIcon;
  group->UserIcon = UserIcon;
  group->Flags = Flags;

  proto_get_caption(group->Caption, &group->CaptionSize,
                    SUPLA_CHANNELGROUP_CAPTION_MAXSIZE);
}

std::list<dcpair> supla_client_channelgroup::get_channel_list(void) {
  std::list<dcpair> result;

  supla_client_channelgroup_relation *rel = NULL;
  safe_array_lock(relarr);

  for (int a = 0; a < safe_array_count(relarr); a++) {
    rel = static_cast<supla_client_channelgroup_relation *>(
        safe_array_get(relarr, a));
    if (rel) {
      dcpair p(rel->getDeviceId(), rel->getChannelId());
      result.push_back(p);
    }
  }
  safe_array_unlock(relarr);
  return result;
}
