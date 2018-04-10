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
#include "../log.h"
#include "../safearray.h"

supla_client_channelgroup::supla_client_channelgroup(int Id, int LocationID,
                                                     int Func,
                                                     const char *Caption,
                                                     int AltIcon)
    : supla_client_objcontainer_item(Id, Caption) {
  this->LocationID = LocationID;
  this->Func = Func;
  this->AltIcon = AltIcon;
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

bool supla_client_channelgroup::remote_update_is_possible(void) { return true; }
