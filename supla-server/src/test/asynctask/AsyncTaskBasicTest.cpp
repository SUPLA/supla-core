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

#include <asynctask/AsyncTaskBasicTest.h>

#include "log.h"

namespace testing {

using std::shared_ptr;
using std::weak_ptr;

AsyncTaskBasicTest::AsyncTaskBasicTest(void) : AsyncTaskTest() {
  task = nullptr;
}
AsyncTaskBasicTest::~AsyncTaskBasicTest(void) {}

void AsyncTaskBasicTest::SetUp() {
  AsyncTaskTest::SetUp();

  if (pool) {
    task = new AsyncTaskMock(queue, pool);
  }
}

void AsyncTaskBasicTest::TearDown() {
  AsyncTaskTest::TearDown();

  if (task) {
    delete task;
    task = nullptr;
  }
}

TEST_F(AsyncTaskBasicTest, removeTask) {
  task->set_delay_usec(1000000);

  shared_ptr<supla_abstract_asynctask> t = task->start();
  EXPECT_EQ(t.use_count(), 2);

  queue->remove_task(t);
  EXPECT_EQ(t.use_count(), 1);
  task = nullptr;
}

TEST_F(AsyncTaskBasicTest, priorityCheck) {
  ASSERT_EQ(task->get_priority(), 0);
  task->set_priority(-123);
  ASSERT_EQ(task->get_priority(), -123);
  shared_ptr<supla_abstract_asynctask> t = task->start();
  task->set_priority(-1);
  ASSERT_EQ(task->get_priority(), -123);
  task = nullptr;
}

TEST_F(AsyncTaskBasicTest, queueGetter) { ASSERT_EQ(task->get_queue(), queue); }

TEST_F(AsyncTaskBasicTest, poolGetter) { ASSERT_EQ(task->get_pool(), pool); }

TEST_F(AsyncTaskBasicTest, defaultState) {
  ASSERT_EQ(task->get_state(), supla_asynctask_state::INIT);
}

TEST_F(AsyncTaskBasicTest, delay) {
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

TEST_F(AsyncTaskBasicTest, timeout) {
  ASSERT_EQ(task->get_timeout(), (unsigned long long)0);
  task->set_timeout(5000);
  ASSERT_EQ(task->get_timeout(), (unsigned long long)5000);
  task->set_timeout(0);
  ASSERT_EQ(task->get_timeout(), (unsigned long long)0);
}

TEST_F(AsyncTaskBasicTest, cancel) {
  ASSERT_NE(task->get_state(), supla_asynctask_state::CANCELED);
  ASSERT_FALSE(task->is_finished());
  task->cancel();
  ASSERT_EQ(task->get_state(), supla_asynctask_state::CANCELED);
  ASSERT_TRUE(task->is_finished());
}

}  // namespace testing
