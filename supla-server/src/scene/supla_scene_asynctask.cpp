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

#include "supla_scene_asynctask.h"

#include <assert.h>

supla_scene_asynctask::supla_scene_asynctask(
    int user_id, int scene_id, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_action_executor *action_executor,
    supla_abstract_value_getter *value_getter,
    supla_scene_operations *operations)
    : supla_abstract_asynctask(queue, pool) {
  assert(action_executor);
  assert(operations);
  assert(value_getter);

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

int supla_scene_asynctask::get_scene_id(void) { return scene_id; }

bool supla_scene_asynctask::_execute(bool *execute_again) {
  supla_scene_operation *operation = operations->pop();
  if (operation) {
    action_executor->execute_action(user_id, operation->get_action(),
                                    value_getter);
    delete operation;
  }

  if (operations->count()) {
    set_delay();
    *execute_again = true;
    return false;
  }

  *execute_again = false;
  return true;
}
