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

#include <list>

#include "client/client_scene_change_indicator.h"
#include "scene/scene_asynctask.h"
#include "scene/scene_search_condition.h"

using std::list;

supla_client_scenes::supla_client_scenes(
    supla_abstract_dobject_remote_updater *updater,
    supla_abstract_client_scene_dao *dao, supla_asynctask_queue *queue)
    : supla_dobjects(updater) {
  assert(dao != NULL);
  this->dao = dao;
  this->queue = queue;

  if (queue) {
    queue->add_observer(this);
  }
}

supla_client_scenes::~supla_client_scenes() {
  if (queue) {
    queue->remove_observer(this);
  }
}

void supla_client_scenes::load(int user_id, int client_id) {
  lock();
  clear();
  list<supla_client_scene *> scenes = dao->get_all_scenes(user_id, client_id);

  for (auto it = scenes.begin(); it != scenes.end(); ++it) {
    if (queue) {
      supla_scene_search_condition cnd(user_id, (*it)->get_id(), false);
      queue->access_task(&cnd, [it](supla_abstract_asynctask *task) -> void {
        supla_scene_asynctask *scene_task =
            dynamic_cast<supla_scene_asynctask *>(task);
        if (scene_task) {
          (*it)->set_state(scene_task->get_scene_state());
        }
      });
    }

    add(*it);
  }

  unlock();
}

void supla_client_scenes::on_asynctask_started_finished(
    supla_abstract_asynctask *asynctask) {
  bool do_update = false;
  supla_scene_asynctask *scene_task =
      dynamic_cast<supla_scene_asynctask *>(asynctask);
  if (scene_task) {
    access_object(scene_task->get_scene_id(),
                  [scene_task, &do_update](supla_dobject *object) -> void {
                    dynamic_cast<supla_client_scene *>(object)->set_state(
                        scene_task->get_scene_state());
                    if (object->get_change_indicator()->is_changed()) {
                      do_update = true;
                    }
                  });
  }

  if (do_update) {
    update_remote();
  }
}

void supla_client_scenes::on_asynctask_started(
    supla_abstract_asynctask *asynctask) {
  on_asynctask_started_finished(asynctask);
}

void supla_client_scenes::on_asynctask_finished(
    supla_abstract_asynctask *asynctask) {
  on_asynctask_started_finished(asynctask);
}

void supla_client_scenes::set_caption(int scene_id, const char *caption) {
  access_object(scene_id, [caption](supla_dobject *object) -> void {
    dynamic_cast<supla_client_scene *>(object)->set_caption(caption);
  });
}

void supla_client_scenes::connection_will_close(void) {
  if (queue) {
    queue->remove_observer(this);
  }
}
