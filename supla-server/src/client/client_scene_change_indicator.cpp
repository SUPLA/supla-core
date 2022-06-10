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

#include "client/client_scene_change_indicator.h"

supla_client_scene_change_indicator::supla_client_scene_change_indicator()
    : supla_dobject_change_indicator() {
  scene_changed = true;
  state_changed = true;
  set_changed(true);
}

supla_client_scene_change_indicator::supla_client_scene_change_indicator(
    bool changed)
    : supla_dobject_change_indicator() {
  scene_changed = changed;
  state_changed = changed;
  set_changed(changed);
}

supla_client_scene_change_indicator::supla_client_scene_change_indicator(
    bool scene_changed, bool state_changed)
    : supla_dobject_change_indicator() {
  this->scene_changed = scene_changed;
  this->state_changed = state_changed;
  set_changed(scene_changed || state_changed);
}

supla_client_scene_change_indicator::~supla_client_scene_change_indicator() {}

bool supla_client_scene_change_indicator::is_scene_changed(void) const {
  return scene_changed;
}

bool supla_client_scene_change_indicator::is_state_changed(void) const {
  return state_changed;
}
