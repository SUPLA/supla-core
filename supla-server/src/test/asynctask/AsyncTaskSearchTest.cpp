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

#include <asynctask/AsyncTaskSearchTest.h>

#include <memory>
#include <vector>

#include "doubles/asynctask/AsyncTaskMock.h"
#include "doubles/integration/asynctask/ChannelOrientedAsyncTaskMock.h"
#include "log.h"

namespace testing {

using std::shared_ptr;
using std::vector;

AsyncTaskSearchTest::AsyncTaskSearchTest(void) {
  queue = NULL;
  pool = NULL;
  cnd = NULL;
}
AsyncTaskSearchTest::~AsyncTaskSearchTest(void) {}

void AsyncTaskSearchTest::SetUp() {
  queue = new supla_asynctask_queue();
  ASSERT_TRUE(queue != NULL);

  pool = new AsyncTaskThreadPoolMock(queue);
  EXPECT_TRUE(pool != NULL);

  cnd = new ChannelSearchCondition();
  EXPECT_TRUE(cnd != NULL);
}

void AsyncTaskSearchTest::TearDown() {
  if (queue) {
    delete queue;
  }

  if (cnd) {
    delete cnd;
  }
}

TEST_F(AsyncTaskSearchTest, testsRelatedToSearchingForTasksInTheQueue) {
  ASSERT_EQ(queue->total_count(), (unsigned int)0);

  int a;

  vector<shared_ptr<supla_abstract_asynctask>> tasks;

  for (a = 0; a < 10; a++) {
    AsyncTaskMock *task = new AsyncTaskMock(queue, pool);
    task->set_delay_usec(1000000);
    tasks.push_back(task->start());
  }

  ChannelOrientedAsyncTaskMock *ctask = NULL;
  ChannelOrientedAsyncTaskMock *ctask_dup = NULL;

  for (a = 0; a < 10; a++) {
    ctask = new ChannelOrientedAsyncTaskMock(queue, pool);
    ctask->set_channel_id(a + 1);
    ctask->set_delay_usec(1000000);
    tasks.push_back(ctask->start());
  }

  ctask_dup = new ChannelOrientedAsyncTaskMock(queue, pool);
  ctask_dup->set_channel_id(ctask->get_channel_id());
  ctask_dup->set_delay_usec(1000000);
  tasks.push_back(ctask_dup->start());

  EXPECT_EQ(queue->total_count(), (unsigned int)21);

  supla_asynctask_state state;

  EXPECT_FALSE(queue->get_task_state(&state, cnd));
  cnd->set_channels({100, 200});
  EXPECT_FALSE(queue->get_task_state(&state, cnd));

  cnd->set_channels({ctask->get_channel_id()});
  EXPECT_TRUE(queue->get_task_state(&state, cnd));
  EXPECT_EQ(state, supla_asynctask_state::WAITING);

  EXPECT_EQ(ctask->get_state(), supla_asynctask_state::WAITING);
  EXPECT_EQ(ctask_dup->get_state(), supla_asynctask_state::WAITING);

  queue->cancel_tasks(cnd);

  EXPECT_EQ(ctask->get_state(), supla_asynctask_state::CANCELED);
  EXPECT_EQ(ctask_dup->get_state(), supla_asynctask_state::CANCELED);

  EXPECT_EQ(queue->get_task_count(cnd), (unsigned int)0);
  cnd->set_channels({1, 2, 3, 4, 5, 700});
  EXPECT_EQ(queue->get_task_count(cnd), (unsigned int)5);
  cnd->set_any_id(true);
  EXPECT_EQ(queue->get_task_count(cnd), (unsigned int)9);
}

}  // namespace testing
