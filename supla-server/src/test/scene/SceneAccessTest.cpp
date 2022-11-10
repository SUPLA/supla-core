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

#include "scene/SceneAccessTest.h"

#include "scene/scene_search_condition.h"

namespace testing {

TEST_F(SceneAccessTest, accessNonExistent) {
  supla_scene_search_condition cnd(1, 2, false);

  bool result =
      queue->access_task(&cnd, [](supla_abstract_asynctask *task) -> void {});
  EXPECT_FALSE(result);

  delete action_executor;
  delete value_getter;
  delete operations;
}

TEST_F(SceneAccessTest, accessSuccessfully) {
  supla_scene_search_condition cnd(1, 2, false);

  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  op->set_delay_ms(500);
  operations->push(op);

  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 1, 2, 0, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);

  bool result =
      queue->access_task(&cnd, [scene](supla_abstract_asynctask *task) -> void {
        EXPECT_TRUE(scene == task);
      });

  EXPECT_TRUE(result);

  // Note that testing with Valgrind turned on increases the time between tasks
  // significantly.

  WaitForState(scene, supla_asynctask_state::SUCCESS, 1000000);
}

} /* namespace testing */
