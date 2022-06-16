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

#include <asynctask/AsyncTaskMainTest.h>

#include "log.h"

namespace testing {

AsyncTaskMainTest::AsyncTaskMainTest(void) : AsyncTaskTest() {}

AsyncTaskMainTest::~AsyncTaskMainTest(void) {}

TEST_F(AsyncTaskMainTest, releaseQueueContainingUninitializedTask) {
  ASSERT_EQ(queue->total_count(), (unsigned int)0);
  ASSERT_EQ(queue->waiting_count(), (unsigned int)0);
  ASSERT_FALSE(pool->is_terminated());

  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);

  ASSERT_EQ(queue->total_count(), (unsigned int)1);
  ASSERT_EQ(queue->waiting_count(), (unsigned int)0);
  ASSERT_FALSE(pool->is_terminated());
  ASSERT_EQ(task->get_state(), supla_asynctask_state::INIT);

  delete queue;
  queue = NULL;
}

TEST_F(AsyncTaskMainTest, runTaskWithoutDelay) {
  pool->set_thread_count_limit(10);

  ASSERT_EQ(pool->thread_count(), (unsigned int)0);
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);
  task->set_job_time_usec(500000);
  task->set_result(true);
  task->set_waiting();
  WaitForState(task, supla_asynctask_state::EXECUTING, 1000000);
  EXPECT_EQ(pool->thread_count(), (unsigned int)1);
  EXPECT_EQ(pool->exec_count(), (unsigned int)0);
  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);
  EXPECT_LT(task->exec_delay_usec(), 200000);
  usleep(1000);
  EXPECT_EQ(pool->thread_count(), (unsigned int)0);
  EXPECT_EQ(pool->exec_count(), (unsigned int)1);
  EXPECT_EQ(pool->highest_number_of_threads(), (unsigned int)1);
}

TEST_F(AsyncTaskMainTest, runTaskWithDelay) {
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);
  task->set_job_time_usec(10000);
  task->set_delay_usec(1200000);
  task->set_result(true);
  task->set_waiting();
  WaitForState(task, supla_asynctask_state::SUCCESS, 2000000);
  EXPECT_GT(task->exec_delay_usec(), 1200000);
  EXPECT_LT(task->exec_delay_usec(), 1500000);
}

TEST_F(AsyncTaskMainTest, runMultipleTasks) {
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

TEST_F(AsyncTaskMainTest, runMultipleTasksWithTwoPools) {
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

TEST_F(AsyncTaskMainTest, priorityTest) {
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
    EXPECT_EQ((*it)->get_state(), supla_asynctask_state::SUCCESS);
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
    EXPECT_EQ((*it)->get_state(), supla_asynctask_state::SUCCESS);
    EXPECT_TRUE(time_usec > (*it)->exec_time_since(&now));
    time_usec = (*it)->exec_time_since(&now);
  }
}

TEST_F(AsyncTaskMainTest, taskWithSubTasks) {
  AsyncTaskMock *task = new AsyncTaskMock(queue, pool, (unsigned int)0, false);
  ASSERT_TRUE(task != NULL);
  task->set_job_time_usec(500000);
  task->set_delay_usec(1000000);
  task->set_job_count_left(3);
  task->set_result(true);
  task->set_waiting();

  for (unsigned int a = 1; a <= 3; a++) {
    supla_log(LOG_DEBUG, "SubTask: %i", a);
    WaitForState(task, supla_asynctask_state::EXECUTING, 2000000);
    WaitForState(task,
                 a == 3 ? supla_asynctask_state::SUCCESS
                        : supla_asynctask_state::WAITING,
                 2000000);
    EXPECT_EQ(task->exec_count(), a);
    EXPECT_EQ(pool->exec_count(), a);
  }
}

}  // namespace testing
