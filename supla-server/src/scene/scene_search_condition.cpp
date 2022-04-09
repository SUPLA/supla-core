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

#include "scene/scene_search_condition.h"

#include "actions/action_gate_openclose.h"
#include "scene/scene_asynctask.h"

supla_scene_search_condition::supla_scene_search_condition(
    int user_id, int scene_id, bool include_sub_tasks) {
  this->user_id = user_id;
  this->scene_id = scene_id;
  this->include_sub_tasks = include_sub_tasks;
}

bool supla_scene_search_condition::condition_met(
    supla_abstract_asynctask *task) {
  supla_scene_asynctask *scene = dynamic_cast<supla_scene_asynctask *>(task);

  if (scene) {
    if (scene->get_user_id() == user_id && scene->get_scene_id() == scene_id) {
      return true;
    }

    return include_sub_tasks &&
           scene->get_caller_id().find(ctScene, scene->get_scene_id(), ctScene,
                                       scene_id);
  }

  if (include_sub_tasks) {
    supla_action_gate_openclose *oc_task =
        dynamic_cast<supla_action_gate_openclose *>(task);
    return oc_task && oc_task->get_caller().find(ctScene, scene_id);
  }

  return false;
}
