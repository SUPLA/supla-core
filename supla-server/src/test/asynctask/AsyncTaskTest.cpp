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

using std::shared_ptr;

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

void AsyncTaskTest::WaitForState(shared_ptr<supla_abstract_asynctask> task,
                                 const supla_asynctask_state &expected,
                                 unsigned int usec) {
  if (!task) {
    return;
  }

  supla_asynctask_state last;
  unsigned int steps = usec / 100;

  for (unsigned int a = 0; a < steps; a++) {
    last = task->get_state();
    if (last == expected) {
      return;
    }
    usleep(100);
  }

  EXPECT_TRUE(last == expected);
  if (last != expected) {
    supla_log(LOG_DEBUG, "last(%i) != expected(%i)", last.get_state(),
              expected.get_state());
  }
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
}  // namespace testing
