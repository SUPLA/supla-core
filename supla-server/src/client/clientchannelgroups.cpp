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

#include "client.h"
#include "../database.h"
#include "../safearray.h"
#include "clientchannelgroup.h"
#include "clientchannelgroups.h"

supla_client_channelgroups::supla_client_channelgroups(supla_client *client)
    : supla_client_objcontainer(client) {}

void supla_client_channelgroups::_load(database *db, e_objc_scope scope) {
  db->get_client_channel_groups(getClient()->getID(), this);
}

void supla_client_channelgroups::_update(supla_client_objcontainer_item *obj,
                                         supla_client_objcontainer_item *source,
                                         e_objc_scope scope) {
  supla_client_channelgroup *cg =
      dynamic_cast<supla_client_channelgroup *>(obj);
  supla_client_channelgroup *src =
      dynamic_cast<supla_client_channelgroup *>(source);
  if (cg && src) {
    cg->update(src);
  }
}

supla_client_objcontainer_item *supla_client_channelgroups::new_item(
    supla_client_objcontainer_item *obj, e_objc_scope scope) {
  supla_client_channelgroup *cg =
      dynamic_cast<supla_client_channelgroup *>(obj);
  if (cg) {
    return new supla_client_channelgroup(cg);
  }

  return NULL;
}

bool supla_client_channelgroups::get_data_for_remote(
    supla_client_objcontainer_item *obj, void **data, bool full, bool EOL,
    bool *check_more) {
  return false;
}

void supla_client_channelgroups::send_data_to_remote_and_free(void *srpc,
                                                              void *data,
                                                              bool full) {}
