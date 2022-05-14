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

#include "scene/scene_operations_dao.h"

#include <list>

#include "scene/scene_db.h"

supla_scene_operations_dao::supla_scene_operations_dao()
    : supla_abstract_scene_operations_dao() {}

supla_scene_operations_dao::~supla_scene_operations_dao() {}

supla_scene_operations *supla_scene_operations_dao::get_scene_operations(
    int scene_id) {
  supla_scene_operations *operations = new supla_scene_operations();
  if (!operations) {
    return NULL;
  }

  supla_scene_db *db = new supla_scene_db();
  if (db) {
    if (db->connect()) {
      std::list<supla_scene_operation *> ops = db->get_operations(scene_id);

      for (auto it = ops.begin(); it != ops.end(); ++it) {
        operations->push(*it);
      }
    }
    delete db;
    db = NULL;
  }

  return operations;
}
