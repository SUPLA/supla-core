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

#include "doubles/scene/SceneActionExecutorMock.h"

#include <scene/scene_asynctask.h>

#include "actions/action_gate_openclose.h"
#include "log.h"

namespace testing {

using std::shared_ptr;

SceneActionExecutorMock::SceneActionExecutorMock() : ActionExecutorMock() {
  this->queue = NULL;
  this->pool = NULL;
  this->property_getter = NULL;
  this->action_executor = NULL;
  this->operations = NULL;
  this->last_executed_asynctask = NULL;
}

SceneActionExecutorMock::~SceneActionExecutorMock() {
  if (operations) {
    delete operations;
  }
}

void SceneActionExecutorMock::execute(void) {
  ActionExecutorMock::execute();
  if (get_scene_id() && queue && pool && action_executor,
      property_getter && operations) {
    last_executed_asynctask =
        (new supla_scene_asynctask(get_caller(), get_user_id(), get_scene_id(),
                                   0, queue, pool, action_executor,
                                   property_getter, operations))
            ->start();
    operations = NULL;
  }
}

void SceneActionExecutorMock::open(void) {
  ActionExecutorMock::open();

  if (get_channel_id() && queue && pool && property_getter) {
    last_executed_asynctask =
        (new supla_action_gate_openclose(
             get_caller(), queue, pool, action_executor, property_getter, NULL,
             get_user_id(), get_device_id(), get_channel_id(), 10000000, true))
            ->start();
  }
}

void SceneActionExecutorMock::set_asynctask_params(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    SceneActionExecutorMock *action_executor,
    supla_abstract_channel_property_getter *property_getter,
    supla_scene_operations *operations) {
  if (this->operations) {
    delete this->operations;
  }

  this->queue = queue;
  this->pool = pool;
  this->action_executor = action_executor;
  this->property_getter = property_getter;
  this->operations = operations;
}

shared_ptr<supla_abstract_asynctask>
SceneActionExecutorMock::get_last_executed_asynctask(void) {
  return last_executed_asynctask;
}

}  // namespace testing
