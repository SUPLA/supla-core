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

#include "srpc_adapter.h"

#include "srpc.h"

supla_srpc_adapter::supla_srpc_adapter(void *srpc)
    : supla_abstract_srpc_adapter(srpc) {}

supla_srpc_adapter::~supla_srpc_adapter() {}

void supla_srpc_adapter::set_proto_version(unsigned char version) {
  srpc_set_proto_version(get_srpc(), version);
}

unsigned char supla_srpc_adapter::get_proto_version(void) {
  return srpc_get_proto_version(get_srpc());
}

_supla_int_t supla_srpc_adapter::dcs_async_set_activity_timeout_result(
    TSDC_SuplaSetActivityTimeoutResult *result) {
  return srpc_dcs_async_set_activity_timeout_result(get_srpc(), result);
}

_supla_int_t supla_srpc_adapter::sc_async_scene_pack_update(
    TSC_SuplaScenePack *scene_pack) {
  return srpc_sc_async_scene_pack_update(get_srpc(), scene_pack);
}

_supla_int_t supla_srpc_adapter::sc_async_scene_state_pack_update(
    TSC_SuplaSceneStatePack *scene_state_pack) {
  return srpc_sc_async_scene_state_pack_update(get_srpc(), scene_state_pack);
}
