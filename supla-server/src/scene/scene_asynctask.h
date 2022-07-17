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

#ifndef SUPLA_SCENE_ASYNCTASK_H_
#define SUPLA_SCENE_ASYNCTASK_H_

#include <scene/scene_operations.h>

#include "actions/action_executor.h"
#include "asynctask/abstract_asynctask.h"
#include "caller.h"
#include "scene/scene_state.h"

enum _sceneExecutionResult_e { serNotExists, serOK, serIsDuringExecution };

class supla_scene_asynctask : public supla_abstract_asynctask {
 private:
  supla_caller caller;
  int user_id;
  int scene_id;
  supla_abstract_action_executor *action_executor;
  supla_abstract_value_getter *value_getter;
  supla_scene_operations *operations;
  void set_delay(void);
  unsigned int op_get_delay_ms(void);
  unsigned int op_get_time_left_ms(void);
  supla_scene_operation *op_pop(void);
  int op_count(void);

 protected:
  bool _execute(bool *execute_again);

 public:
  supla_scene_asynctask(const supla_caller &caller, int user_id, int scene_id,
                        supla_asynctask_queue *queue,
                        supla_abstract_asynctask_thread_pool *pool,
                        supla_abstract_action_executor *action_executor,
                        supla_abstract_value_getter *value_getter,
                        supla_scene_operations *operations,
                        bool release_immediately);
  virtual ~supla_scene_asynctask();
  const supla_caller &get_caller(void) const;
  int get_user_id(void);
  int get_scene_id(void);
  supla_scene_state get_scene_state(void);
  static bool get_scene_state(supla_asynctask_queue *queue, int user_id,
                              int scene_id, supla_scene_state *state);

  static supla_asynctask_queue *get_queue(void);
  static supla_abstract_asynctask_thread_pool *get_pool(void);

  static _sceneExecutionResult_e execute(
      supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
      const supla_caller &caller, int user_id, int scene_id);
  static void interrupt(supla_asynctask_queue *queue, int user_id,
                        int scene_id);
};

#endif /* SUPLA_SCENE_ASYNCTASK_H_ */
