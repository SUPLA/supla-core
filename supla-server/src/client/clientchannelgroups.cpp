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
#include "../safearray.h"
#include "clientchannelgroups.h"

supla_client_channelgroups::supla_client_channelgroups(supla_client *client)
    : supla_client_objcontainer(client) {}

void supla_client_channelgroups::_load(database *db) {}

void supla_client_channelgroups::_update(
    supla_client_objcontainer_item *obj,
    supla_client_objcontainer_item *source) {}

supla_client_objcontainer_item *supla_client_channelgroups::new_item(
    supla_client_objcontainer_item *obj) {
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
