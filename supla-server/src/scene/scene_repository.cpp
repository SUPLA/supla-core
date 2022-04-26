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
    : supla_dobject_repository(user_id) {
  this->current = NULL;
  this->db = NULL;
}

supla_scene_repository::~supla_scene_repository() {}

supla_scene_db *supla_scene_repository::db_connect(void) {
  if (!db) {
    db = new supla_scene_db();
  }
  if (db && !db->connect()) {
    delete db;
    db = NULL;
  }
  return db;
}

supla_dobject *supla_scene_repository::get_first(void) {
  supla_scene_db *db = db_connect();
  if (db) {
    db->close_scene_query();
    db->open_scene_query(get_user_id());
    current = db->fetch_scene();
    if (!current) {
      db->close_scene_query();
    }
  } else {
    current = NULL;
  }

  return current;
}

supla_dobject *supla_scene_repository::get_next(void) {
  if (!current) {
    return get_first();
  }
  if (db) {
    current = db->fetch_scene();
    if (!current) {
      db->close_scene_query();
    }
  } else {
    current = NULL;
  }

  return current;
}

supla_dobject *supla_scene_repository::get_object(int id) {
  current = NULL;
  supla_dobject *result = NULL;
  supla_scene_db *db = db_connect();
  if (db) {
    db->close_scene_query();
    db->open_scene_query(get_user_id(), id);
    result = db->fetch_scene();
    db->close_scene_query();
  }

  return result;
}
