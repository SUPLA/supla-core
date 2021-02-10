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

#include "AsyncTaskIntegrationTest.h"

namespace testing {

AsyncTaskIntegrationTest::AsyncTaskIntegrationTest(void) {
  queue = NULL;
  pool = NULL;
}
AsyncTaskIntegrationTest::~AsyncTaskIntegrationTest(void) {}

void AsyncTaskIntegrationTest::SetUp() {
  queue = new supla_asynctask_queue();
  ASSERT_TRUE(queue != NULL);

  pool = new AsyncTaskThreadPoolMock(queue);
  EXPECT_TRUE(pool != NULL);
}

void AsyncTaskIntegrationTest::TearDown() {
  if (queue) {
    delete queue;
  }
}

TEST_F(AsyncTaskIntegrationTest, releaseQueueContainingUninitializedTask) {
  ASSERT_EQ(queue->total_count(), (unsigned int)0);
  ASSERT_EQ(queue->waiting_count(), (unsigned int)0);
  ASSERT_FALSE(pool->is_terminated());

  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);

  ASSERT_EQ(queue->total_count(), (unsigned int)1);
  ASSERT_EQ(queue->waiting_count(), (unsigned int)0);
  ASSERT_FALSE(pool->is_terminated());
  ASSERT_EQ(task->get_state(), STA_STATE_INIT);

  delete queue;
  queue = NULL;
}

TEST_F(AsyncTaskIntegrationTest, runTaskWithoutDelay) {
  ASSERT_EQ(pool->thread_count(), (unsigned int)0);
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);
  task->set_job_time_usec(100000);
  task->set_waiting();
  usleep(1000);
  EXPECT_EQ(pool->thread_count(), (unsigned int)1);
  EXPECT_EQ(pool->exec_count(), (unsigned int)0);
  EXPECT_EQ(task->get_state(), STA_STATE_EXECUTING);

  delete task;
}

}  // namespace testing
