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

#include "scene/scene_asynctask.h"

#include <assert.h>
#include <unistd.h>

#include "asynctask/asynctask_default_thread_pool.h"
#include "asynctask/asynctask_queue.h"
#include "device/value_getter.h"
#include "log.h"
#include "scene/scene_operations_dao.h"
#include "scene/scene_search_condition.h"

supla_scene_asynctask::supla_scene_asynctask(
    const supla_caller &caller, int user_id, int scene_id,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_action_executor *action_executor,
    supla_abstract_value_getter *value_getter,
    supla_scene_operations *operations, bool release_immediately)
    : supla_abstract_asynctask(queue, pool, 0, release_immediately) {
  assert(action_executor);
  assert(operations);
  assert(value_getter);

  this->caller = caller;
  this->user_id = user_id;
  this->scene_id = scene_id;
  this->action_executor = action_executor;
  this->value_getter = value_getter;
  this->operations = operations;

  set_delay();
  set_waiting();
}

supla_scene_asynctask::~supla_scene_asynctask() {
  if (action_executor) {
    delete action_executor;
    action_executor = NULL;
  }

  if (operations) {
    delete operations;
    operations = NULL;
  }

  if (value_getter) {
    delete value_getter;
    value_getter = NULL;
  }
}

void supla_scene_asynctask::set_delay(void) {
  set_delay_usec(operations->get_delay_ms() * 1000);
}

const supla_caller &supla_scene_asynctask::get_caller(void) const {
  return caller;
}

int supla_scene_asynctask::get_user_id(void) { return user_id; }

int supla_scene_asynctask::get_scene_id(void) { return scene_id; }

bool supla_scene_asynctask::_execute(bool *execute_again) {
  do {
    supla_scene_operation *operation = operations->pop();
    if (operation) {
      if (operation->get_action_config()->get_subject_type() != stScene ||
          !caller.find(ctScene,
                       operation->get_action_config()->get_subject_id())) {
        action_executor->execute_action(supla_caller(caller, ctScene, scene_id),
                                        user_id, operation->get_action_config(),
                                        value_getter);
      }

      delete operation;
    }

    if (!operations->count() || operations->get_delay_ms() > 1000) {
      break;
    } else {
      // Operations to be performed not later than in a second should be
      // performed in one _execute cycle.
      usleep(operations->get_delay_ms() * 1000);
    }
  } while (true);

  if (operations->count()) {
    set_delay();
    *execute_again = true;
    return false;
  }

  *execute_again = false;
  return true;
}

// static
supla_asynctask_queue *supla_scene_asynctask::get_queue(void) {
  return supla_asynctask_queue::global_instance();
}

// static
supla_abstract_asynctask_thread_pool *supla_scene_asynctask::get_pool(void) {
  return supla_asynctask_default_thread_pool::global_instance();
}

// static
_sceneExecutionResult_e supla_scene_asynctask::execute(
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    const supla_caller &caller, int user_id, int scene_id) {
  supla_scene_search_condition cnd(user_id, scene_id, false);
  if (queue->task_exists(&cnd)) {
    return serIsDuringExecution;
  } else {
    supla_scene_operations_dao dao;
    supla_scene_operations *operations = dao.get_scene_operations(scene_id);
    if (operations && operations->count() == 0) {
      delete operations;
      operations = NULL;
    }

    if (operations == NULL) {
      return serNotExists;
    } else {
      supla_action_executor *action_executor = new supla_action_executor();
      supla_value_getter *value_getter = new supla_value_getter();

      new supla_scene_asynctask(caller, user_id, scene_id, queue, pool,
                                action_executor, value_getter, operations,
                                true);
      return serOK;
    }
  }
}

// static
void supla_scene_asynctask::interrupt(supla_asynctask_queue *queue, int user_id,
                                      int scene_id) {
  supla_scene_search_condition cnd(user_id, scene_id, true);
  queue->cancel_tasks(&cnd);
}
