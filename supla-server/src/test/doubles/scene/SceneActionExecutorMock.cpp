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

#include <scene/scene_asynctask.h>
#include "doubles/scene/SceneActionExecutorMock.h"

#include "log.h"

namespace testing {

SceneActionExecutorMock::SceneActionExecutorMock() : ActionExecutorMock() {
  this->queue = NULL;
  this->pool = NULL;
  this->value_getter = NULL;
  this->action_executor = NULL;
  this->operations = NULL;
  this->last_executed_scene = NULL;
}

SceneActionExecutorMock::~SceneActionExecutorMock() {
  if (operations) {
    delete operations;
  }
}

void SceneActionExecutorMock::execute(void) {
  ActionExecutorMock::execute();
  if (get_scene_id() && queue && pool && action_executor,
      value_getter && operations) {
    last_executed_scene = new supla_scene_asynctask(
        get_caller(), 1, get_scene_id(), queue, pool, action_executor,
        value_getter, operations, false);
    operations = NULL;
  }
}

void SceneActionExecutorMock::set_scene_params(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    SceneActionExecutorMock *action_executor,
    supla_abstract_value_getter *value_getter,
    supla_scene_operations *operations) {
  if (this->operations) {
    delete this->operations;
  }

  this->queue = queue;
  this->pool = pool;
  this->action_executor = action_executor;
  this->value_getter = value_getter;
  this->operations = operations;
}

supla_scene_asynctask *SceneActionExecutorMock::get_last_executed_scene(void) {
  return last_executed_scene;
}

}  // namespace testing
