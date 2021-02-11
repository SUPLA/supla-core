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
#include "log.h"

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

void AsyncTaskIntegrationTest::WaitForState(AsyncTaskMock *task,
                                            async_task_state expected,
                                            unsigned int usec) {
  async_task_state last;
  unsigned int steps = usec / 100;
  for (unsigned int a = 0; a < steps; a++) {
    last = task->get_state();
    if (last == expected) {
      return;
    }
    usleep(100);
  }

  ASSERT_EQ(last, expected);
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
  pool->set_thread_count_limit(10);

  ASSERT_EQ(pool->thread_count(), (unsigned int)0);
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);
  task->set_job_time_usec(500000);
  task->set_result(true);
  task->set_waiting();
  WaitForState(task, STA_STATE_EXECUTING, 1000000);
  EXPECT_EQ(pool->thread_count(), (unsigned int)1);
  EXPECT_EQ(pool->exec_count(), (unsigned int)0);
  WaitForState(task, STA_STATE_SUCCESS, 1000000);
  usleep(1000);
  EXPECT_EQ(pool->thread_count(), (unsigned int)0);
  EXPECT_EQ(pool->exec_count(), (unsigned int)1);
  EXPECT_EQ(pool->highest_number_of_threads(), (unsigned int)1);
}

}  // namespace testing
