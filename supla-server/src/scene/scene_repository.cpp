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

#include "scene/scene_repository.h"

supla_scene_repository::supla_scene_repository(int user_id)
    : supla_scene_abstract_repository(user_id) {}

supla_scene_repository::~supla_scene_repository() {}

supla_scene_db *supla_scene_repository::get_db(void) {
  supla_scene_db *db = new supla_scene_db();
  if (db && !db->connect()) {
    delete db;
    db = NULL;
  }

  return db;
}

supla_scene *supla_scene_repository::get_scene(int id) {
  supla_scene *result = NULL;
  supla_scene_db *db = get_db();
  if (db) {
    result = db->get_scene(get_user_id(), id);
    delete db;
    db = NULL;
  }

  return result;
}

std::vector<supla_scene *> supla_scene_repository::get_all_scenes(void) {
  std::vector<supla_scene *> result;
  supla_scene_db *db = get_db();
  if (db) {
    result = db->get_all_scenes(get_user_id());
    delete db;
    db = NULL;
  }

  return result;
}
