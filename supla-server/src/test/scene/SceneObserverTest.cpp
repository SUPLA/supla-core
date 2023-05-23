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

#include "scene/SceneObserverTest.h"

namespace testing {

using std::shared_ptr;

SceneObserverTest::~SceneObserverTest() {
  start_count = 0;
  finish_count = 0;
  finished_at = {};
}

void SceneObserverTest::SetUp() {
  SceneTest::SetUp();
  start_count = 0;
  finish_count = 0;
  finished_at = {};
  queue->add_observer(this);
}

void SceneObserverTest::TearDown() {
  queue->remove_observer(this);
  SceneTest::TearDown();
}

void SceneObserverTest::on_asynctask_started(
    supla_abstract_asynctask *asynctask) {
  if (dynamic_cast<supla_scene_asynctask *>(asynctask)) {
    start_count++;
  }
}

void SceneObserverTest::on_asynctask_finished(
    supla_abstract_asynctask *asynctask) {
  if (dynamic_cast<supla_scene_asynctask *>(asynctask)) {
    finish_count++;
    gettimeofday(&finished_at, NULL);
  }
}

TEST_F(SceneObserverTest, startAndFinish) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  op->set_delay_ms(1000);
  operations->push(op);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  op->set_delay_ms(200);
  operations->push(op);

  EXPECT_EQ(start_count, 0);
  EXPECT_EQ(finish_count, 0);

  shared_ptr<supla_abstract_asynctask> scene =
      (new supla_scene_asynctask(supla_caller(ctIPC), 1, 2, 0, queue, pool,
                                 action_executor, property_getter, operations))
          ->start();

  EXPECT_EQ(start_count, 1);
  EXPECT_EQ(finish_count, 0);

  // Note that testing with Valgrind turned on increases the time between tasks
  // significantly.

  WaitForState(scene, supla_asynctask_state::SUCCESS, 1500000);

  EXPECT_EQ(start_count, 1);
  EXPECT_EQ(finish_count, 1);

  long long diff = (finished_at.tv_sec * 1000000 + finished_at.tv_usec) -
                   (now.tv_sec * 1000000 + now.tv_usec);

  EXPECT_GE(diff, 1200000);
  EXPECT_LE(diff, 1400000);
}

TEST_F(SceneObserverTest, removeObserverBeforeTaskFinished) {
  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  op->set_delay_ms(1000);
  operations->push(op);

  EXPECT_EQ(start_count, 0);
  EXPECT_EQ(finish_count, 0);

  shared_ptr<supla_abstract_asynctask> scene =
      (new supla_scene_asynctask(supla_caller(ctIPC), 1, 2, 0, queue, pool,
                                 action_executor, property_getter, operations))
          ->start();

  EXPECT_EQ(start_count, 1);
  EXPECT_EQ(finish_count, 0);

  queue->remove_observer(this);

  // Note that testing with Valgrind turned on increases the time between tasks
  // significantly.

  WaitForState(scene, supla_asynctask_state::SUCCESS, 1500000);

  EXPECT_EQ(start_count, 1);
  EXPECT_EQ(finish_count, 0);
}

} /* namespace testing */
