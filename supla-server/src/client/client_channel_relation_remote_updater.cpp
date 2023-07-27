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

#include "client/client_channel_relation_remote_updater.h"

#include "device/channel_relation.h"
#include "srpc/srpc_adapter.h"

supla_client_channel_relation_remote_updater::
    supla_client_channel_relation_remote_updater(
        supla_abstract_srpc_adapter *srpc_adapter)
    : supla_abstract_dobject_remote_updater(srpc_adapter) {
  relation_pack = nullptr;
}

supla_client_channel_relation_remote_updater::
    ~supla_client_channel_relation_remote_updater() {}

bool supla_client_channel_relation_remote_updater::is_protocol_version_allowed(
    int protocol_version) {
  return protocol_version >= 21;
}

bool supla_client_channel_relation_remote_updater::pre_transaction_verification(
    supla_dobject *object, bool first) {
  return true;
}

bool supla_client_channel_relation_remote_updater::on_transaction_begin(
    supla_dobject *object, int protocol_version) {
  if (!relation_pack) {
    relation_pack = new TSC_SuplaChannelRelationPack();
  }

  if (relation_pack) {
    *relation_pack = {};
    return true;
  }

  return false;
}

bool supla_client_channel_relation_remote_updater::prepare_the_update(
    supla_dobject *object,
    supla_dobject_change_indicator **new_change_indicator,
    bool *transaction_should_end) {
  supla_channel_relation *relation =
      dynamic_cast<supla_channel_relation *>(object);
  if (!relation) {
    return false;
  }

  bool result = false;

  if (relation_pack) {
    result = supla_srpc_adapter::datapack_add<TSC_SuplaChannelRelationPack,
                                              TSC_SuplaChannelRelation>(
        relation_pack, SUPLA_CHANNEL_RELATION_PACK_MAXCOUNT,
        [relation](TSC_SuplaChannelRelation *sc_relation) -> void {
          relation->convert(sc_relation);
        });

    if (result) {
      *new_change_indicator = new supla_dobject_change_indicator(false);
    }
  }

  return result;
}

void supla_client_channel_relation_remote_updater::on_transaction_end(
    supla_abstract_srpc_adapter *srpc_adapter) {
  if (relation_pack) {
    if (relation_pack->count) {
      supla_srpc_adapter::datapack_set_eol<TSC_SuplaChannelRelationPack>(
          relation_pack);
      srpc_adapter->sc_async_channel_relation_pack_update(relation_pack);
    }

    delete relation_pack;
    relation_pack = NULL;
  }
}
