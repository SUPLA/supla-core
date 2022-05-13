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

#include "scene/scenes.h"

#include <assert.h>

#include "scene/scene_asynctask.h"
#include "scene/scene_search_condition.h"

supla_scenes::supla_scenes(
    supla_scene_abstract_repository *repository, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_action_executor_factory *action_executor_factory,
    supla_abstract_value_getter_factory *value_getter_factory)
    : supla_dobjects() {
  assert(repository != NULL);
  assert(queue != NULL);
  assert(pool != NULL);
  assert(action_executor_factory != NULL);
  assert(value_getter_factory != NULL);

  this->repository = repository;
  this->queue = queue;
  this->pool = pool;
  this->action_executor_factory = action_executor_factory;
  this->value_getter_factory = value_getter_factory;
}

supla_scenes::~supla_scenes(void) { delete repository; }

void supla_scenes::load() {
  lock();

  const std::vector<supla_scene *> scenes = repository->get_all_scenes();

  for (auto it = scenes.begin(); it != scenes.end(); ++it) {
    add_or_replace(*it);
  }
  unlock();
}

void supla_scenes::load(int id) {
  interrupt(id);
  add_or_replace(repository->get_scene(id));
}

void supla_scenes::access_scene(
    int id, std::function<void(supla_scene *scene)> on_access) {
  access_object(id, [on_access](supla_dobject *object) -> void {
    supla_scene *scene = dynamic_cast<supla_scene *>(object);
    if (scene) {
      on_access(scene);
    }
  });
}

void supla_scenes::execute(const supla_caller &caller, int id) {
  lock();
  supla_scene_search_condition cnd(repository->get_user_id(), id, false);
  if (queue->task_exists(&cnd)) {
  } else {
    access_scene(id, [caller, id, this](supla_scene *scene) -> void {
      supla_scene_operations *operations = scene->get_operations()->clone();
      new supla_scene_asynctask(
          caller, repository->get_user_id(), id, queue, pool,
          action_executor_factory->get_action_executor(),
          value_getter_factory->get_value_getter(), operations, true);
    });
  }
  unlock();
}

void supla_scenes::interrupt(int id) {
  supla_scene_search_condition cnd(repository->get_user_id(), id, false);
  queue->cancel_tasks(&cnd);
}
