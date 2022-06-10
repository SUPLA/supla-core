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

#include "log.h"

supla_client_scene_remote_updater::supla_client_scene_remote_updater(
    supla_abstract_srpc_adapter *srpc_adapter)
    : supla_abstract_dobject_remote_updater(srpc_adapter) {}

supla_client_scene_remote_updater::~supla_client_scene_remote_updater() {}

bool supla_client_scene_remote_updater::on_transaction_begin(
    supla_dobject *object, int protocol_version) {
  return true;
}

bool supla_client_scene_remote_updater::prepare_the_update(
    supla_dobject *object,
    supla_dobject_change_indicator **new_change_indicator,
    bool *transaction_should_end) {
  return true;
}

void supla_client_scene_remote_updater::on_transaction_end(
    supla_abstract_srpc_adapter *srpc_adapter) {}
