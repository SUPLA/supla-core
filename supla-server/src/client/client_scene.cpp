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

#include "client/client_scene.h"

#include <stdlib.h>
#include <string.h>

#include "client/client_scene_change_indicator.h"
#include "proto.h"

supla_client_scene::supla_client_scene(int id) : supla_dobject(id) {
  this->user_icon_id = 0;
  this->alt_icon_id = 0;
  this->caption = NULL;
  this->location_id = 0;

  this->set_change_indicator(new supla_client_scene_change_indicator());
}

supla_client_scene::~supla_client_scene() {
  if (caption) {
    free(caption);
    caption = NULL;
  }
}

void supla_client_scene::set_caption(const char *caption) {
  if ((this->caption == nullptr && caption == nullptr) ||
      (this->caption != nullptr && caption != nullptr &&
       strncmp(this->caption, caption, SUPLA_SCENE_CAPTION_MAXSIZE) == 0)) {
    return;
  }
  if (this->caption) {
    free(this->caption);
    this->caption = NULL;
  }

  if (caption) {
    this->caption = strndup(caption, SUPLA_SCENE_CAPTION_MAXSIZE);
  }

  const supla_client_scene_change_indicator *ind =
      dynamic_cast<const supla_client_scene_change_indicator *>(
          get_change_indicator());
  if (!ind || !ind->is_scene_changed()) {
    set_change_indicator(new supla_client_scene_change_indicator(
        true, ind && ind->is_state_changed()));
  }
}

const char *supla_client_scene::get_caption(void) { return caption; }

void supla_client_scene::set_user_icon_id(int user_icon_id) {
  this->user_icon_id = user_icon_id;
}

int supla_client_scene::get_user_icon_id(void) { return user_icon_id; }

void supla_client_scene::set_location_id(int location_id) {
  this->location_id = location_id;
}

int supla_client_scene::get_location_id(void) { return location_id; }

void supla_client_scene::set_alt_icon_id(int alt_icon_id) {
  this->alt_icon_id = alt_icon_id;
}

int supla_client_scene::get_alt_icon_id(void) { return alt_icon_id; }

const supla_scene_state &supla_client_scene::get_state(void) { return state; }

void supla_client_scene::set_state(const supla_scene_state &state) {
  if (this->state != state) {
    this->state = state;
    const supla_client_scene_change_indicator *ind =
        dynamic_cast<const supla_client_scene_change_indicator *>(
            get_change_indicator());
    if (!ind || !ind->is_state_changed()) {
      set_change_indicator(new supla_client_scene_change_indicator(
          ind && ind->is_scene_changed(), true));
    }
  }
}

void supla_client_scene::convert(TSC_SuplaScene *dest) {
  *dest = {};

  dest->Id = get_id();
  dest->LocationId = get_location_id();
  dest->AltIcon = get_alt_icon_id();
  dest->UserIcon = get_user_icon_id();

  sproto__set_null_terminated_string(get_caption(), dest->Caption,
                                     &dest->CaptionSize,
                                     SUPLA_SCENE_CAPTION_MAXSIZE);
}

void supla_client_scene::convert(TSC_SuplaSceneState *dest) {
  *dest = {};

  state.convert(dest);
  dest->SceneId = get_id();
}
