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

#include "scene/scene_db.h"

#include <mysql.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

supla_scene_db::supla_scene_db(void) : svrdb() {}

supla_scene_db::~supla_scene_db(void) { close_scene_query(); }

void supla_scene_db::open_scene_query(int user_id, int scene_id) {}

void supla_scene_db::open_scene_query(int user_id) {
  open_scene_query(user_id, 0);
}

supla_scene *supla_scene_db::fetch_scene(void) { return NULL; }

void supla_scene_db::close_scene_query(void) {}
