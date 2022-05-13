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

#ifndef SUPLA_SCENES_H_
#define SUPLA_SCENES_H_

#include <scene/scene_abstract_repository.h>

#include "actions/abstract_action_executor_factory.h"
#include "asynctask/asynctask_queue.h"
#include "caller.h"
#include "device/abstract_value_getter_factory.h"
#include "distributedobjects/dobjects.h"

class supla_scenes : public supla_dobjects {
 private:
  supla_scene_abstract_repository *repository;
  supla_asynctask_queue *queue;
  supla_abstract_asynctask_thread_pool *pool;
  supla_abstract_action_executor_factory *action_executor_factory;
  supla_abstract_value_getter_factory *value_getter_factory;

 protected:
  void access_scene(int id, std::function<void(supla_scene *scene)> on_access);

 public:
  explicit supla_scenes(
      supla_scene_abstract_repository *repository, supla_asynctask_queue *queue,
      supla_abstract_asynctask_thread_pool *pool,
      supla_abstract_action_executor_factory *action_executor_factory,
      supla_abstract_value_getter_factory *value_getter_factory);
  virtual ~supla_scenes(void);
  void load();
  void load(int id);
  void execute(const supla_caller &caller, int id);
  void interrupt(int id);
};

#endif /* SUPLA_SCENES_H_ */
