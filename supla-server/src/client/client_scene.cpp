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

#include "proto.h"

supla_client_scene::supla_client_scene(int id) : supla_dobject(id) {
  this->user_icon_id = 0;
  this->alt_icon_id = 0;
  this->caption = NULL;
  this->location_id = 0;

  this->initiator_id = 0;
  this->initiator_name = 0;
  this->milliseconds_from_start = 0;
  this->milliseconds_left = 0;
}

supla_client_scene::~supla_client_scene() {
  if (caption) {
    free(caption);
    caption = NULL;
  }

  if (initiator_name) {
    free(initiator_name);
    initiator_name = NULL;
  }
}

void supla_client_scene::set_caption(const char *caption) {
  if (this->caption) {
    free(this->caption);
    this->caption = NULL;
  }

  if (caption) {
    this->caption = strndup(caption, SUPLA_SCENE_CAPTION_MAXSIZE);
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

void supla_client_scene::set_initiator_id(int initiator_id) {
  this->initiator_id = initiator_id;
}

int supla_client_scene::get_initiator_id(void) { return initiator_id; }

void supla_client_scene::set_initiator_name(const char *initiator_name) {
  if (this->initiator_name) {
    free(this->initiator_name);
    this->initiator_name = NULL;
  }

  if (initiator_name) {
    this->initiator_name =
        strndup(initiator_name, SUPLA_INITIATOR_NAME_MAXSIZE);
  }
}

const char *supla_client_scene::get_initiator_name(void) {
  return initiator_name;
}

void supla_client_scene::set_milliseconds_from_start(
    unsigned _supla_int_t milliseconds_from_start) {
  this->milliseconds_from_start = milliseconds_from_start;
}

unsigned _supla_int_t supla_client_scene::get_milliseconds_from_start(void) {
  return milliseconds_from_start;
}

void supla_client_scene::set_milliseconds_left(
    unsigned _supla_int_t milliseconds_left) {
  this->milliseconds_left = milliseconds_left;
}

unsigned _supla_int_t supla_client_scene::get_milliseconds_left(void) {
  return milliseconds_left;
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

  dest->SceneId = get_id();
  dest->MillisecondsFromStart = get_milliseconds_from_start();
  dest->MillisecondsLeft = get_milliseconds_left();

  dest->InitiatorId = get_initiator_id();
  sproto__set_null_terminated_string(get_initiator_name(), dest->InitiatorName,
                                     &dest->InitiatorNameSize,
                                     SUPLA_INITIATOR_NAME_MAXSIZE);
}
