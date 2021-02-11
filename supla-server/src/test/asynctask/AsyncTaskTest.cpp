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

#include "AsyncTaskTest.h"
#include "log.h"

namespace testing {

AsyncTaskTest::AsyncTaskTest(void) {
  queue = NULL;
  pool = NULL;
  task = NULL;
}
AsyncTaskTest::~AsyncTaskTest(void) {}

void AsyncTaskTest::SetUp() {
  queue = new supla_asynctask_queue();
  ASSERT_TRUE(queue != NULL);

  pool = new AsyncTaskThreadPoolMock(queue);
  EXPECT_TRUE(pool != NULL);

  if (pool) {
    task = new AsyncTaskMock(queue, pool, -123, true);
  }
}

void AsyncTaskTest::TearDown() { delete queue; }

TEST_F(AsyncTaskTest, initWithNulls_1) {
  ASSERT_DEATH(new AsyncTaskMock(NULL, NULL, 0, true),
               "Assertion `queue' failed");
}

TEST_F(AsyncTaskTest, initWithNulls_2) {
  EXPECT_DEATH(new AsyncTaskMock(queue, NULL, 0, true),
               "Assertion `pool' failed");
}

TEST_F(AsyncTaskTest, correctInitialization_1) {
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, 0, true);
  ASSERT_TRUE(task != NULL);
  delete task;
}

TEST_F(AsyncTaskTest, correctInitialization_2) {
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool);
  ASSERT_TRUE(task != NULL);
  EXPECT_EQ(task->get_priority(), 0);
  EXPECT_TRUE(task->release_immediately_after_execution());
  delete task;
}

TEST_F(AsyncTaskTest, priorityCheck) { ASSERT_EQ(task->get_priority(), -123); }

TEST_F(AsyncTaskTest, queueGetter) { ASSERT_EQ(task->get_queue(), queue); }

TEST_F(AsyncTaskTest, poolGetter) { ASSERT_EQ(task->get_pool(), pool); }

TEST_F(AsyncTaskTest, defaultState) {
  ASSERT_EQ(task->get_state(), STA_STATE_INIT);
}

TEST_F(AsyncTaskTest, releaseFlag) {
  ASSERT_TRUE(task->release_immediately_after_execution());

  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, 0, false);
  ASSERT_TRUE(task != NULL);
  ASSERT_FALSE(task->release_immediately_after_execution());
  delete task;
}

TEST_F(AsyncTaskTest, delay) {
  ASSERT_EQ(task->get_delay_usec(), 0);
  ASSERT_EQ(task->time_left_usec(NULL), 0);
  usleep(1000);
  ASSERT_EQ(task->time_left_usec(NULL), 0);
  task->set_delay_usec(2000000);
  ASSERT_EQ(task->get_delay_usec(), 2000000);
  ASSERT_GT(task->time_left_usec(NULL), 1990000);
  task->set_delay_usec(1);
  usleep(1000);
  ASSERT_LT(task->time_left_usec(NULL), -990);
}

TEST_F(AsyncTaskTest, timeout) {
  ASSERT_EQ(task->get_timeout(), (unsigned long long)0);
  task->set_timeout(5000);
  ASSERT_EQ(task->get_timeout(), (unsigned long long)5000);
  task->set_timeout(0);
  ASSERT_EQ(task->get_timeout(), (unsigned long long)0);
}

TEST_F(AsyncTaskTest, cancel) {
  ASSERT_NE(task->get_state(), STA_STATE_CANCELED);
  ASSERT_FALSE(task->is_finished());
  task->cancel();
  ASSERT_EQ(task->get_state(), STA_STATE_CANCELED);
  ASSERT_TRUE(task->is_finished());
}
}  // namespace testing
