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

#include "scene/SceneTest.h"

#include "log.h"  // NOLINT

namespace testing {

SceneTest::SceneTest() : AsyncTaskTest() {}

SceneTest::~SceneTest() {}

void SceneTest::SetUp() {
  AsyncTaskTest::SetUp();

  action_executor = new ActionExecutorMock();
  ASSERT_FALSE(action_executor == NULL);

  value_getter = new ValueGetterStub();
  if (!value_getter) {
    delete action_executor;
    action_executor = NULL;
  }
  ASSERT_FALSE(value_getter == NULL);

  operations = new supla_scene_operations();
  if (!operations) {
    delete action_executor;
    action_executor = NULL;

    delete value_getter;
    value_getter = NULL;
  }

  ASSERT_FALSE(operations == NULL);
}

TEST_F(SceneTest, executeEmptyScene) {
  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 1, 2, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);
  WaitForState(scene, STA_STATE_SUCCESS, 1000);
  EXPECT_EQ(pool->exec_count(), (unsigned int)1);
}

} /* namespace testing */
