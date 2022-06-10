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

#include "client/client_scene_remote_updater.h"

#include "client/client_scene.h"
#include "client/client_scene_change_indicator.h"
#include "srpc_adapter.h"

supla_client_scene_remote_updater::supla_client_scene_remote_updater(
    supla_abstract_srpc_adapter *srpc_adapter)
    : supla_abstract_dobject_remote_updater(srpc_adapter) {
  scene_pack = NULL;
  state_pack = NULL;
}

supla_client_scene_remote_updater::~supla_client_scene_remote_updater() {}

bool supla_client_scene_remote_updater::on_transaction_begin(
    supla_dobject *object, int protocol_version) {
  if (protocol_version < 18) {
    return false;
  }

  const supla_client_scene_change_indicator *ind =
      dynamic_cast<const supla_client_scene_change_indicator *>(
          object->get_change_indicator());

  if (ind) {
    if (ind->is_scene_changed()) {
      if (!scene_pack) {
        scene_pack = new TSC_SuplaScenePack();
      }

      if (scene_pack) {
        *scene_pack = {};
        return true;
      }
    } else if (ind->is_state_changed()) {
      if (!state_pack) {
        state_pack = new TSC_SuplaSceneStatePack();
      }

      if (state_pack) {
        *state_pack = {};
        return true;
      }
    }
  }

  return false;
}

bool supla_client_scene_remote_updater::prepare_the_update(
    supla_dobject *object,
    supla_dobject_change_indicator **new_change_indicator,
    bool *transaction_should_end) {
  const supla_client_scene_change_indicator *ind =
      dynamic_cast<const supla_client_scene_change_indicator *>(
          object->get_change_indicator());
  if (!ind) {
    return false;
  }

  supla_client_scene *scene = dynamic_cast<supla_client_scene *>(object);
  if (!scene) {
    return false;
  }

  if (scene_pack) {
    if (ind->is_scene_changed()) {
      *new_change_indicator = new supla_client_scene_change_indicator(
          false, ind->is_state_changed());

      supla_srpc_adapter::datapack_add<TSC_SuplaScenePack, TSC_SuplaScene>(
          scene_pack, SUPLA_SCENE_PACK_MAXCOUNT,
          [scene](TSC_SuplaScene *sc_scene) -> void {
            scene->convert(sc_scene);
          });

      return true;
    }
  } else if (state_pack) {
    if (ind->is_state_changed()) {
      *new_change_indicator = new supla_client_scene_change_indicator(
          ind->is_scene_changed(), false);

      supla_srpc_adapter::datapack_add<TSC_SuplaSceneStatePack,
                                       TSC_SuplaSceneState>(
          state_pack, SUPLA_SCENE_STATE_PACK_MAXCOUNT,
          [scene](TSC_SuplaSceneState *sc_state) -> void {
            scene->convert(sc_state);
          });
    }

    return true;
  }

  return false;
}

void supla_client_scene_remote_updater::on_transaction_end(
    supla_abstract_srpc_adapter *srpc_adapter) {
  if (scene_pack) {
    if (scene_pack->count) {
      supla_srpc_adapter::datapack_set_eol<TSC_SuplaScenePack>(scene_pack);
      srpc_adapter->sc_async_scene_pack_update(scene_pack);
    }

    delete scene_pack;
    scene_pack = NULL;
  }

  if (state_pack) {
    if (state_pack->count) {
      supla_srpc_adapter::datapack_set_eol<TSC_SuplaSceneStatePack>(state_pack);
      srpc_adapter->sc_async_scene_state_pack_update(state_pack);
    }
    delete state_pack;
    state_pack = NULL;
  }
}
