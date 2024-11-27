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

#include "client/client_channel_state_remote_updater.h"

#include "device/channel_state.h"
#include "srpc/srpc_adapter.h"

supla_client_channel_state_remote_updater::
    supla_client_channel_state_remote_updater(
        supla_abstract_srpc_adapter *srpc_adapter)
    : supla_abstract_dobject_remote_updater(srpc_adapter) {
  state_pack = nullptr;
  first = nullptr;
}

supla_client_channel_state_remote_updater::
    ~supla_client_channel_state_remote_updater() {}

bool supla_client_channel_state_remote_updater::is_protocol_version_allowed(
    int protocol_version) {
  return protocol_version >= 26;
}

bool supla_client_channel_state_remote_updater::pre_transaction_verification(
    supla_dobject *object, bool first) {
  if (first) {
    this->first = object;
  }
  return true;
}

bool supla_client_channel_state_remote_updater::on_transaction_begin(
    supla_dobject *object, int protocol_version) {
  if (!state_pack) {
    state_pack = new TSC_SuplaChannelStatePack();
  }

  if (state_pack) {
    *state_pack = {};
    return true;
  }

  return false;
}

bool supla_client_channel_state_remote_updater::prepare_the_update(
    supla_dobject *object,
    supla_dobject_change_indicator **new_change_indicator,
    bool *transaction_should_end) {
  supla_channel_state *state = dynamic_cast<supla_channel_state *>(object);
  if (!state) {
    return false;
  }

  bool result = false;

  if (state_pack) {
    result = supla_srpc_adapter::datapack_add<TSC_SuplaChannelStatePack,
                                              TDSC_ChannelState>(
        state_pack, SUPLA_CHANNEL_STATE_PACK_MAXCOUNT, object == first,
        [state](TDSC_ChannelState *sc_state) -> void {
          *sc_state = *state->get_state();
        });

    if (result) {
      *new_change_indicator = new supla_dobject_change_indicator(false);
    }
  }

  return result;
}

void supla_client_channel_state_remote_updater::on_transaction_end(
    supla_abstract_srpc_adapter *srpc_adapter) {
  if (state_pack) {
    if (state_pack->count) {
      supla_srpc_adapter::datapack_set_eol<TSC_SuplaChannelStatePack>(
          state_pack, true);
      srpc_adapter->sc_async_channel_state_pack_update(state_pack);
    }

    delete state_pack;
    state_pack = nullptr;
  }

  first = nullptr;
}
