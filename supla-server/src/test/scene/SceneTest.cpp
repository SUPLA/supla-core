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

#include "TestHelper.h"
#include "actions/action_gate_openclose.h"
#include "log.h"  // NOLINT
#include "scene/scene_search_condition.h"

namespace testing {

SceneTest::SceneTest() : AsyncTaskTest() {
  action_executor = NULL;
  value_getter = NULL;
  operations = NULL;
}

SceneTest::~SceneTest() {}

void SceneTest::SetUp() {
  AsyncTaskTest::SetUp();

  pool->set_thread_count_limit(10);

  action_executor = new SceneActionExecutorMock();
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
  EXPECT_EQ(operations->get_delay_ms(), 0);

  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 1, 2, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);
  WaitForState(scene, supla_asynctask_state::SUCCESS, 1000);
  WaitForExec(pool, 1, 1000);
}

TEST_F(SceneTest, executeSceneWithoutDelay) {
  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  supla_action_config action_config;
  action_config.set_action_id(ACTION_TURN_ON);
  action_config.set_subject_id(10);
  action_config.set_subject_type(stChannel);

  op->set_action_config(action_config);
  operations->push(op);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  action_config.set_action_id(ACTION_TURN_OFF);
  op->set_action_config(action_config);
  operations->push(op);

  EXPECT_EQ(operations->get_delay_ms(), 0);

  struct timeval now = {};
  gettimeofday(&now, NULL);

  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 1, 2, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);

  // Note that testing with Valgrind turned on increases the time between tasks
  // significantly.

  WaitForState(scene, supla_asynctask_state::SUCCESS, 2000);
  WaitForExec(pool, 1,
              2000000);  // Both operations should be performed in one cycle.
                         // There should be one cycle in total.

  std::list<struct timeval> times = action_executor->getTimes();
  EXPECT_EQ(times.size(), 2UL);
  EXPECT_LT(TestHelper::timeDiffUs(times.back(), now) -
                TestHelper::timeDiffUs(times.front(), now),
            100000UL);
  EXPECT_LT(TestHelper::timeDiffUs(times.back(), now), 100000UL);
}

TEST_F(SceneTest, executeSceneWithDelayBetweenActions) {
  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  supla_action_config action_config;
  action_config.set_action_id(ACTION_TURN_ON);
  action_config.set_subject_id(10);
  action_config.set_subject_type(stChannel);

  op->set_delay_ms(100);
  op->set_action_config(action_config);
  operations->push(op);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  action_config.set_action_id(ACTION_TURN_OFF);
  op->set_action_config(action_config);
  operations->push(op);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  action_config.set_action_id(ACTION_OPEN);
  action_config.set_subject_id(123);
  op->set_delay_ms(1200);
  op->set_action_config(action_config);
  operations->push(op);

  EXPECT_EQ(operations->get_delay_ms(), 100);

  struct timeval now = {};
  gettimeofday(&now, NULL);

  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 1, 2, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);

  // Note that testing with Valgrind turned on increases the time between tasks
  // significantly.

  WaitForState(scene, supla_asynctask_state::SUCCESS, 1500000);
  WaitForExec(pool, 2,
              2000000);  // Two of the three operations should be performed in
                         // one cycle. There should be two cycles in total.

  std::list<struct timeval> times = action_executor->getTimes();
  EXPECT_EQ(times.size(), 3UL);
  EXPECT_LT(TestHelper::timeDiffUs(times.back(), now) -
                TestHelper::timeDiffUs(times.front(), now),
            1500000UL);
  EXPECT_GT(TestHelper::timeDiffUs(times.front(), now), 90000UL);
  EXPECT_LT(TestHelper::timeDiffUs(times.front(), now), 150000UL);
  EXPECT_LT(TestHelper::timeDiffUs(times.back(), now), 1500000UL);
  EXPECT_EQ(action_executor->getOnCounter(), 1);
  EXPECT_EQ(action_executor->getOffCounter(), 1);
  EXPECT_EQ(action_executor->getOpenCounter(), 1);
  EXPECT_EQ(action_executor->counterSetCount(), 3);
}

TEST_F(SceneTest, executeSceneInsideScene) {
  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  supla_action_config action_config;
  action_config.set_action_id(ACTION_EXECUTE);
  action_config.set_subject_id(15);
  action_config.set_subject_type(stScene);

  op->set_action_config(action_config);
  operations->push(op);

  EXPECT_EQ(operations->get_delay_ms(), 0);

  // Parameters for the second scene
  supla_scene_operations *operations_s2 = new supla_scene_operations();
  ASSERT_FALSE(operations_s2 == NULL);

  ValueGetterStub *value_getter_s2 = new ValueGetterStub();
  ASSERT_FALSE(value_getter_s2 == NULL);

  SceneActionExecutorMock *action_executor_s2 = new SceneActionExecutorMock();

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  action_config.set_action_id(ACTION_TURN_ON);
  action_config.set_subject_id(10);
  action_config.set_subject_type(stChannel);

  op->set_action_config(action_config);
  operations_s2->push(op);

  action_executor->set_asynctask_params(queue, pool, action_executor_s2,
                                        value_getter_s2, operations_s2);
  // ----------------------------

  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 2, 111, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);

  WaitForState(scene, supla_asynctask_state::SUCCESS, 1000);
  WaitForState(action_executor->get_last_executed_asynctask(),
               supla_asynctask_state::SUCCESS, 1000);
  EXPECT_GT(pool->exec_count(), (unsigned int)0);
  EXPECT_EQ(action_executor->getExecuteCounter(), 1);
  EXPECT_EQ(action_executor_s2->getOnCounter(), 1);
  WaitForExec(pool, 2, 1000000);
  EXPECT_EQ(action_executor_s2->get_caller().stack_size(), 3);
  EXPECT_TRUE(action_executor_s2->get_caller().find(ctIPC));
  EXPECT_TRUE(action_executor_s2->get_caller().find(ctScene, 111));
  EXPECT_TRUE(action_executor_s2->get_caller().find(ctScene, 15));
  // action_executor_s2->get_caller().print_stack();
}

TEST_F(SceneTest, infinityLoop) {
  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);

  supla_action_config action_config;
  action_config.set_action_id(ACTION_EXECUTE);
  action_config.set_subject_id(15);
  action_config.set_subject_type(stScene);

  op->set_action_config(action_config);
  operations->push(op);

  // Parameters for the second scene
  supla_scene_operations *operations_s2 = new supla_scene_operations();
  ASSERT_FALSE(operations_s2 == NULL);

  ValueGetterStub *value_getter_s2 = new ValueGetterStub();
  ASSERT_FALSE(value_getter_s2 == NULL);

  SceneActionExecutorMock *action_executor_s2 = new SceneActionExecutorMock();

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  action_config.set_subject_id(111);
  op->set_action_config(action_config);
  operations_s2->push(op);

  action_executor->set_asynctask_params(queue, pool, action_executor_s2,
                                        value_getter_s2, operations_s2);
  // ----------------------------

  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 2, 111, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);

  WaitForState(scene, supla_asynctask_state::SUCCESS, 1000);
  WaitForState(action_executor->get_last_executed_asynctask(),
               supla_asynctask_state::SUCCESS, 1000);
  EXPECT_GT(pool->exec_count(), (unsigned int)0);
  EXPECT_EQ(action_executor->getExecuteCounter(), 1);
  EXPECT_EQ(action_executor_s2->getExecuteCounter(), 0);
  EXPECT_EQ(action_executor_s2->counterSetCount(), 0);
}

TEST_F(SceneTest, interruptScene) {
  if (!supla_user::find(2, false)) {
    new supla_user(2, "aa4f2278a5dbdcdb4fa52278dbdca54f", "");
  }

  supla_scene_operation *op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  supla_action_config action_config;
  action_config.set_action_id(ACTION_EXECUTE);
  action_config.set_subject_id(15);
  action_config.set_subject_type(stScene);
  op->set_delay_ms(0);
  op->set_action_config(action_config);
  operations->push(op);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  action_config.set_action_id(ACTION_TURN_ON);
  action_config.set_subject_id(50);
  action_config.set_subject_type(stChannel);
  op->set_delay_ms(2000);
  op->set_action_config(action_config);
  operations->push(op);

  // Parameters for the second scene
  supla_scene_operations *operations_s2 = new supla_scene_operations();
  ASSERT_FALSE(operations_s2 == NULL);

  ValueGetterStub *value_getter_s2 = new ValueGetterStub();
  ASSERT_FALSE(value_getter_s2 == NULL);

  SceneActionExecutorMock *action_executor_s2 = new SceneActionExecutorMock();
  ASSERT_FALSE(action_executor_s2 == NULL);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  action_config.set_action_id(ACTION_EXECUTE);
  action_config.set_subject_id(16);
  action_config.set_subject_type(stScene);
  op->set_delay_ms(0);
  op->set_action_config(action_config);
  operations_s2->push(op);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  action_config.set_action_id(ACTION_TURN_OFF);
  action_config.set_subject_id(51);
  action_config.set_subject_type(stChannel);
  op->set_delay_ms(20000);
  op->set_action_config(action_config);
  operations_s2->push(op);

  action_executor->set_asynctask_params(queue, pool, action_executor_s2,
                                        value_getter_s2, operations_s2);
  // ----------------------------

  // Parameters for the third scene
  supla_scene_operations *operations_s3 = new supla_scene_operations();
  ASSERT_FALSE(operations_s3 == NULL);

  ValueGetterStub *value_getter_s3 = new ValueGetterStub();
  ASSERT_FALSE(value_getter_s3 == NULL);

  SceneActionExecutorMock *action_executor_s3 = new SceneActionExecutorMock();
  ASSERT_FALSE(action_executor_s3 == NULL);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  action_config.set_action_id(ACTION_OPEN);
  action_config.set_subject_id(52);
  action_config.set_subject_type(stChannel);
  op->set_delay_ms(0);
  op->set_action_config(action_config);
  operations_s3->push(op);

  op = new supla_scene_operation();
  ASSERT_FALSE(op == NULL);
  action_config.set_action_id(ACTION_TURN_OFF);
  action_config.set_subject_id(53);
  action_config.set_subject_type(stChannel);
  op->set_delay_ms(20000);
  op->set_action_config(action_config);
  operations_s3->push(op);

  action_executor_s2->set_asynctask_params(queue, pool, action_executor_s3,
                                           value_getter_s3, operations_s3);

  // Parameters for the Open / Close action

  SceneActionExecutorMock *action_executor_oc = new SceneActionExecutorMock();
  ASSERT_FALSE(action_executor_oc == NULL);

  ValueGetterStub *value_getter_oc = new ValueGetterStub();
  ASSERT_FALSE(value_getter_oc == NULL);

  action_executor_s3->set_asynctask_params(queue, pool, action_executor_oc,
                                           value_getter_oc, NULL);

  // ----------------------------

  supla_scene_asynctask *scene = new supla_scene_asynctask(
      supla_caller(ctIPC), 2, 111, queue, pool, action_executor, value_getter,
      operations, false);
  ASSERT_FALSE(scene == NULL);

  supla_scene_search_condition cnd(2, 15, true);

  WaitForExec(pool, 5, 5000000);
  queue->cancel_tasks(&cnd);
  WaitForState(scene, supla_asynctask_state::SUCCESS, 5000000);
  EXPECT_EQ(action_executor->getOnCounter(), 1);
  EXPECT_EQ(action_executor->getExecuteCounter(), 1);
  EXPECT_EQ(action_executor->counterSetCount(), 2);
  EXPECT_EQ(action_executor_s2->getExecuteCounter(), 1);
  EXPECT_EQ(action_executor_s2->counterSetCount(), 1);
  EXPECT_EQ(action_executor_s3->getOpenCounter(), 1);
  EXPECT_EQ(action_executor_s3->counterSetCount(), 1);

  WaitForState(action_executor->get_last_executed_asynctask(),
               supla_asynctask_state::CANCELED, 1000000);

  WaitForState(action_executor_s2->get_last_executed_asynctask(),
               supla_asynctask_state::CANCELED, 1000000);

  WaitForState(action_executor_s3->get_last_executed_asynctask(),
               supla_asynctask_state::CANCELED, 1000000);

  ASSERT_TRUE(dynamic_cast<supla_action_gate_openclose *>(
                  action_executor_s3->get_last_executed_asynctask()) != NULL);
}

} /* namespace testing */
