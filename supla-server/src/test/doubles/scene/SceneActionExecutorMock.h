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

#ifndef SCENE_ACTION_EXECUTOR_MOCK_H_
#define SCENE_ACTION_EXECUTOR_MOCK_H_

#include <scene/scene_asynctask.h>

#include "asynctask/abstract_asynctask.h"
#include "doubles/actions/ActionExecutorMock.h"

namespace testing {

class SceneActionExecutorMock : public ActionExecutorMock {
 private:
  supla_asynctask_queue *queue;
  supla_abstract_asynctask_thread_pool *pool;
  SceneActionExecutorMock *action_executor;
  supla_abstract_channel_property_getter *property_getter;
  supla_scene_operations *operations;
  std::shared_ptr<supla_abstract_asynctask> last_executed_asynctask;

 public:
  SceneActionExecutorMock();
  virtual ~SceneActionExecutorMock();
  virtual void execute(void);
  virtual void open(void);
  void set_asynctask_params(
      supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
      SceneActionExecutorMock *action_executor,
      supla_abstract_channel_property_getter *property_getter,
      supla_scene_operations *operations);
  std::shared_ptr<supla_abstract_asynctask> get_last_executed_asynctask(void);
};

} /* namespace testing */

#endif /* SCENE_ACTION_EXECUTOR_MOCK_H_ */
