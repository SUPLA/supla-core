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

#include <asynctask/AsyncTaskTest.h>

#include "log.h"

namespace testing {

AsyncTaskTest::AsyncTaskTest(void) {
  queue = NULL;
  pool = NULL;
}
AsyncTaskTest::~AsyncTaskTest(void) {}

void AsyncTaskTest::SetUp() {
  queue = new supla_asynctask_queue();
  ASSERT_TRUE(queue != NULL);

  pool = new AsyncTaskThreadPoolMock(queue);
  EXPECT_TRUE(pool != NULL);
}

void AsyncTaskTest::TearDown() {
  if (queue) {
    delete queue;
  }
}

void AsyncTaskTest::WaitForState(supla_abstract_asynctask *task,
                                 async_task_state expected, unsigned int usec) {
  async_task_state last;
  unsigned int steps = usec / 10000;

  for (unsigned int a = 0; a < steps; a++) {
    last = task->get_state();
    if (last == expected) {
      return;
    }
    usleep(10000);
  }

  ASSERT_EQ(last, expected);
}

void AsyncTaskTest::WaitForExec(AsyncTaskThreadPoolMock *pool,
                                unsigned int expected_count,
                                unsigned int usec) {
  unsigned int steps = usec / 100000;

  for (unsigned int a = 0; a < steps; a++) {
    if (pool->exec_count() == expected_count) {
      return;
    }
    usleep(100000);
  }

  ASSERT_EQ(pool->exec_count(), expected_count);
}

TEST_F(AsyncTaskTest, releaseQueueContainingUninitializedTask) {
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

TEST_F(AsyncTaskTest, runTaskWithoutDelay) {
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
  EXPECT_LT(task->exec_delay_usec(), 200000);
  usleep(1000);
  EXPECT_EQ(pool->thread_count(), (unsigned int)0);
  EXPECT_EQ(pool->exec_count(), (unsigned int)1);
  EXPECT_EQ(pool->highest_number_of_threads(), (unsigned int)1);
}

TEST_F(AsyncTaskTest, runTaskWithDelay) {
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);
  task->set_job_time_usec(10000);
  task->set_delay_usec(1200000);
  task->set_result(true);
  task->set_waiting();
  WaitForState(task, STA_STATE_SUCCESS, 2000000);
  EXPECT_GT(task->exec_delay_usec(), 1200000);
  EXPECT_LT(task->exec_delay_usec(), 1500000);
}

TEST_F(AsyncTaskTest, runMultipleTasks) {
  pool->set_thread_count_limit(10);
  for (int a = 0; a < 50; a++) {
    AsyncTaskMock *task = new AsyncTaskMock(queue, pool);
    task->set_job_time_usec(100000);
    task->set_result(true);
    task->set_waiting();
  }

  WaitForExec(pool, 50, 5000000);
  EXPECT_EQ(pool->highest_number_of_threads(), (unsigned int)10);
}

TEST_F(AsyncTaskTest, runMultipleTasksWithTwoPools) {
  AsyncTaskThreadPoolMock *pool2 = new AsyncTaskThreadPoolMock(queue);
  EXPECT_TRUE(pool2 != NULL);

  pool->set_thread_count_limit(10);
  pool2->set_thread_count_limit(5);

  for (int a = 0; a < 100; a++) {
    AsyncTaskMock *task = new AsyncTaskMock(queue, a % 2 ? pool : pool2);
    task->set_job_time_usec(200000);
    task->set_result(true);
    task->set_waiting();
  }

  WaitForExec(pool, 50, 10000000);
  EXPECT_EQ(pool->highest_number_of_threads(), (unsigned int)10);
  EXPECT_EQ(pool2->highest_number_of_threads(), (unsigned int)5);

  delete pool2;
}

TEST_F(AsyncTaskTest, priorityTest) {
  pool->set_thread_count_limit(1);
  pool->hold();

  std::vector<AsyncTaskMock *> tasks;
  int a;

  for (a = 0; a < 100; a++) {
    AsyncTaskMock *task = new AsyncTaskMock(queue, pool, a, false);
    task->set_job_time_usec(100);
    task->set_result(true);
    task->set_waiting();

    tasks.push_back(task);
  }

  pool->unhold();

  WaitForExec(pool, 100, 20000000);

  struct timeval now;
  gettimeofday(&now, NULL);

  long long time_usec = tasks.back()->exec_time_since(&now);

  for (std::vector<AsyncTaskMock *>::reverse_iterator it = tasks.rbegin() + 1;
       it != tasks.rend(); ++it) {
    EXPECT_TRUE(time_usec > (*it)->exec_time_since(&now));
    EXPECT_EQ((*it)->get_state(), STA_STATE_SUCCESS);
    time_usec = (*it)->exec_time_since(&now);
  }

  tasks.clear();
  pool->hold();

  for (a = 0; a < 100; a++) {
    AsyncTaskMock *task = new AsyncTaskMock(queue, pool, 0, false);
    task->set_job_time_usec(100);
    task->set_result(true);
    task->set_waiting();

    tasks.push_back(task);
  }

  pool->unhold();

  WaitForExec(pool, 200, 20000000);
  gettimeofday(&now, NULL);

  time_usec = tasks.front()->exec_time_since(&now);

  for (std::vector<AsyncTaskMock *>::iterator it = tasks.begin() + 1;
       it != tasks.end(); ++it) {
    EXPECT_EQ((*it)->get_state(), STA_STATE_SUCCESS);
    EXPECT_TRUE(time_usec > (*it)->exec_time_since(&now));
    time_usec = (*it)->exec_time_since(&now);
  }
}

TEST_F(AsyncTaskTest, taskWithSubTasks) {
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);
  task->set_job_time_usec(500000);
  task->set_delay_usec(1000000);
  task->set_job_count_left(3);
  task->set_result(true);
  task->set_waiting();

  for (unsigned int a = 1; a <= 3; a++) {
    supla_log(LOG_DEBUG, "SubTask: %i", a);
    WaitForState(task, STA_STATE_EXECUTING, 2000000);
    WaitForState(task, a == 3 ? STA_STATE_SUCCESS : STA_STATE_WAITING, 2000000);
    EXPECT_EQ(task->exec_count(), a);
    EXPECT_EQ(pool->exec_count(), a);
  }
}

}  // namespace testing
