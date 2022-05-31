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
  this->caption = NULL;
}

supla_client_scene::~supla_client_scene() {
  if (caption) {
    free(caption);
    caption = NULL;
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
