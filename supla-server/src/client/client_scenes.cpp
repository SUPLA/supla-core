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

#include "client/client_scenes.h"

#include <assert.h>

supla_client_scenes::supla_client_scenes(
    supla_abstract_dobject_remote_updater *updater,
    supla_abstract_client_scene_dao *dao)
    : supla_dobjects(updater) {
  assert(dao != NULL);
  this->dao = dao;
}

supla_client_scenes::~supla_client_scenes() {}

void supla_client_scenes::load(int user_id, int client_id, int scene_id) {
  lock();
  remove(scene_id);
  supla_client_scene *scene = dao->get_scene(user_id, client_id, scene_id);
  if (scene) {
    add(scene);
  }
  unlock();
}

void supla_client_scenes::load(int user_id, int client_id) {
  lock();
  clear();
  std::list<supla_client_scene *> scenes =
      dao->get_all_scenes(user_id, client_id);

  for (auto it = scenes.begin(); it != scenes.end(); ++it) {
    add(*it);
  }

  unlock();
}
