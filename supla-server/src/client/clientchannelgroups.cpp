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
#include <stdlib.h>  // NOLINT
#include <string.h>  // NOLINT
#include <list>      // NOLINT
#include "clientchannelgroup.h"
#include "clientchannelgroups.h"
#include "database.h"
#include "log.h"
#include "safearray.h"
#include "srpc.h"
#include "user.h"

supla_client_channelgroups::supla_client_channelgroups(supla_client *client)
    : supla_client_objcontainer(client) {
  id_cmp_use_both[detail1] = true;
}

void supla_client_channelgroups::_load(database *db, e_objc_scope scope) {
  switch (scope) {
    case master:
      db->get_client_channel_groups(getClient()->getID(), this);
      break;
    case detail1:
      db->get_client_channel_group_relations(getClient()->getID(), this);
      break;
    case detail2:
      break;
  }
}

void supla_client_channelgroups::load(void) {
  supla_client_objcontainer::load(master);
  supla_client_objcontainer::load(detail1);
}

bool supla_client_channelgroups::add(supla_client_objcontainer_item *obj,
                                     e_objc_scope scope) {
  bool result = false;
  supla_client_channelgroup *cg = NULL;

  if (scope == master) {
    cg = static_cast<supla_client_channelgroup *>(obj);
    // Add only this groups that remote update is possible
    result = cg && cg->remote_update_is_possible() &&
             supla_client_objcontainer::add(obj, scope);

  } else if (scope == detail1) {
    supla_client_channelgroup_relation *cg_rel =
        static_cast<supla_client_channelgroup_relation *>(obj);
    if (cg_rel) {
      cg = findGroup(cg_rel->getGroupId());
      if (cg) {
        result = supla_client_objcontainer::add(obj, scope);
        if (result) {
          cg->add_relation(cg_rel);
        }
      }
    }

  } else if (scope == detail2) {
    supla_client_channelgroup_value *cg_val =
        static_cast<supla_client_channelgroup_value *>(obj);

    if (channelRelationExists(cg_val->getChannelId())) {
      result = supla_client_objcontainer::add(obj, scope);
    }
  }

  return result;
}

bool supla_client_channelgroups::channelRelationExists(int ChannelId) {
  bool result = false;
  supla_client_channelgroup_relation *cg_rel = NULL;
  void *arr = getArr(detail1);

  safe_array_lock(arr);

  for (int a = 0; a < safe_array_count(arr); a++) {
    cg_rel = static_cast<supla_client_channelgroup_relation *>(
        safe_array_get(arr, a));
    if (cg_rel && cg_rel->getChannelId() == ChannelId) {
      result = true;
      break;
    }
  }

  safe_array_unlock(arr);
  return result;
}

supla_client_channelgroup *supla_client_channelgroups::findGroup(int Id) {
  return static_cast<supla_client_channelgroup *>(find(Id, master));
}

template <typename TSuplaDataPack, class TObjClass>
bool supla_client_channelgroups::get_datapack_for_remote(
    supla_client_objcontainer_item *obj, void **data, int max_count) {
  TSuplaDataPack *pack = static_cast<TSuplaDataPack *>(*data);
  if (pack == NULL) {
    pack = (TSuplaDataPack *)malloc(sizeof(TSuplaDataPack));
    memset(pack, 0, sizeof(TSuplaDataPack));
  }

  *data = pack;

  if (pack->count < max_count) {
    static_cast<TObjClass *>(obj)->proto_get(&pack->items[pack->count]);
    pack->items[pack->count].EOL = 0;
    pack->count++;
    return true;
  } else {
    pack->total_left++;
  }

  return false;
}

template <typename TSuplaDataPack>
void supla_client_channelgroups::set_pack_eol(void *data) {
  TSuplaDataPack *pack = static_cast<TSuplaDataPack *>(data);

  if (pack && pack->count > 0) {
    if (pack->total_left == 0) pack->items[pack->count - 1].EOL = 1;
  }
}

bool supla_client_channelgroups::get_data_for_remote(
    supla_client_objcontainer_item *obj, void **data, bool full,
    bool *check_more, e_objc_scope scope) {
  *check_more = true;

  if (scope == master) {
    return get_datapack_for_remote<TSC_SuplaChannelGroupPack,
                                   supla_client_channelgroup>(
        obj, data, SUPLA_CHANNELGROUP_PACK_MAXCOUNT);
  } else if (scope == detail1) {
    return get_datapack_for_remote<TSC_SuplaChannelGroupRelationPack,
                                   supla_client_channelgroup_relation>(
        obj, data, SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT);
  } else if (scope == detail2) {
    return get_datapack_for_remote<TSC_SuplaChannelValuePack,
                                   supla_client_channelgroup_value>(
        obj, data, SUPLA_CHANNELVALUE_PACK_MAXCOUNT);
  }

  return false;
}

void supla_client_channelgroups::send_data_to_remote_and_free(
    void *srpc, void *data, bool full, e_objc_scope scope) {
  if (scope == master) {
    set_pack_eol<TSC_SuplaChannelGroupPack>(data);

    srpc_sc_async_channelgroup_pack_update(
        srpc, static_cast<TSC_SuplaChannelGroupPack *>(data));

  } else if (scope == detail1) {
    set_pack_eol<TSC_SuplaChannelGroupRelationPack>(data);

    srpc_sc_async_channelgroup_relation_pack_update(
        srpc, static_cast<TSC_SuplaChannelGroupRelationPack *>(data));
  } else if (scope == detail2) {
    set_pack_eol<TSC_SuplaChannelValuePack>(data);

    srpc_sc_async_channelvalue_pack_update(
        srpc, static_cast<TSC_SuplaChannelValuePack *>(data));
  }

  free(data);
}

void supla_client_channelgroups::on_channel_value_changed(void *srpc,
                                                          int DeviceId,
                                                          int ChannelId) {
  on_value_changed(srpc, ChannelId, DeviceId, detail2, OI_REMOTEUPDATE_FULL);
}

bool supla_client_channelgroups::set_device_channel_new_value(
    TCS_SuplaNewValue *new_value) {
  bool result = false;

  std::list<t_dc_pair> pairs;
  void *arr = getArr(master);

  safe_array_lock(arr);
  supla_client_channelgroup *cgroup = findGroup(new_value->Id);
  if (cgroup) {
    pairs = cgroup->get_channel_list();
  }
  safe_array_unlock(arr);

  for (std::list<t_dc_pair>::iterator it = pairs.begin(); it != pairs.end();
       it++) {
    if (getClient()->getUser()->set_device_channel_value(
            0, it->DeviceId, it->ChannelId, new_value->value)) {
      result = true;
    }
  }

  return result;
}
