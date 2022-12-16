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
#include "db/db_access_provider.h"
#include "device/value_getter.h"
#include "log.h"
#include "scene/scene_dao.h"
#include "scene/scene_operations_dao.h"
#include "scene/scene_search_condition.h"

supla_scene_asynctask::supla_scene_asynctask(
    const supla_caller &caller, int user_id, int scene_id,
    unsigned int estimated_execution_time, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_action_executor *action_executor,
    supla_abstract_value_getter *value_getter,
    supla_scene_operations *operations)
    : supla_abstract_asynctask(queue, pool) {
  assert(action_executor);
  assert(operations);
  assert(value_getter);

  this->caller = caller;
  this->user_id = user_id;
  this->scene_id = scene_id;
  this->estimated_execution_time = estimated_execution_time;
  this->action_executor = action_executor;
  this->value_getter = value_getter;
  this->operations = operations;

  set_delay_usec(op_get_delay_ms() * 1000);
  set_observable();
}

supla_scene_asynctask::~supla_scene_asynctask() {
  on_task_finished();

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

unsigned int supla_scene_asynctask::op_get_delay_ms(void) {
  lock();
  int result = operations->get_delay_ms();
  unlock();
  return result;
}

supla_scene_operation *supla_scene_asynctask::op_pop(void) {
  lock();
  supla_scene_operation *result = operations->pop();
  unlock();

  return result;
}

int supla_scene_asynctask::op_count(void) {
  lock();
  int result = operations->count();
  unlock();
  return result;
}

const supla_caller &supla_scene_asynctask::get_caller(void) const {
  return caller;
}

int supla_scene_asynctask::get_user_id(void) { return user_id; }

int supla_scene_asynctask::get_scene_id(void) { return scene_id; }

unsigned int supla_scene_asynctask::get_estimated_execution_time(void) {
  return estimated_execution_time;
}

supla_scene_state supla_scene_asynctask::get_scene_state(void) {
  return supla_scene_state(caller, get_started_at(),
                           is_finished() ? 0 : get_estimated_execution_time());
}

// static
bool supla_scene_asynctask::get_scene_state(supla_asynctask_queue *queue,
                                            int user_id, int scene_id,
                                            supla_scene_state *state) {
  bool success = false;
  supla_scene_search_condition cnd(user_id, scene_id, false);
  queue->access_task(
      &cnd, [&success, &state](supla_abstract_asynctask *task) -> void {
        *state = dynamic_cast<supla_scene_asynctask *>(task)->get_scene_state();
        success = true;
      });

  return success;
}

bool supla_scene_asynctask::_execute(bool *execute_again) {
  do {
    supla_scene_operation *operation = op_pop();
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

    if (!op_count() || op_get_delay_ms() > 1000U) {
      break;
    } else {
      // Operations to be performed not later than in a second should be
      // performed in one _execute cycle.
      usleep(op_get_delay_ms() * 1000U);
    }
  } while (true);

  if (op_count()) {
    set_delay_usec(op_get_delay_ms() * 1000);
    *execute_again = true;
    return false;
  } else {
    // The scene may have ended but the dependent scenes may still be going on,
    // so we wait as long as defined by the state. If we do not do it, client
    // applications will be notified about too early completion of the scene.
    supla_scene_state state = get_scene_state();
    if (state.get_milliseconds_left() > 100) {
      *execute_again = true;
      set_delay_usec(state.get_milliseconds_left() * 1000);
      return false;
    }
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
    const supla_caller &caller, int user_id, int scene_id,
    bool interrupt_before_execute) {
  supla_scene_search_condition cnd(user_id, scene_id, false);
  if (!interrupt_before_execute && queue->task_exists(&cnd)) {
    return serIsDuringExecution;
  } else {
    if (interrupt_before_execute) {
      interrupt(queue, user_id, scene_id);
      usleep(10000);
    }

    supla_db_access_provider dba;
    supla_scene_dao scene_dao(&dba);
    supla_scene_operations_dao op_dao(&dba);
    supla_scene_operations *operations = op_dao.get_scene_operations(scene_id);

    if (operations && operations->count() == 0) {
      delete operations;
      operations = NULL;
    }

    if (operations == NULL) {
      return serNotExists;
    } else {
      supla_action_executor *action_executor = new supla_action_executor();
      supla_value_getter *value_getter = new supla_value_getter();

      supla_scene_asynctask *scene = new supla_scene_asynctask(
          caller, user_id, scene_id,
          scene_dao.get_estimated_execution_time(scene_id), queue, pool,
          action_executor, value_getter, operations);
      scene->start();
      return serOK;
    }
  }
}

// static
void supla_scene_asynctask::interrupt(supla_asynctask_queue *queue, int user_id,
                                      int scene_id) {
  {
    bool is_finished = true;
    supla_scene_search_condition cnd(user_id, scene_id, false);
    queue->access_task(
        &cnd, [&is_finished](supla_abstract_asynctask *task) -> void {
          is_finished =
              dynamic_cast<supla_scene_asynctask *>(task)->is_finished();
        });

    if (is_finished) {
      return;
    }
  }

  supla_scene_search_condition cnd(user_id, scene_id, true);
  queue->cancel_tasks(&cnd);
}
