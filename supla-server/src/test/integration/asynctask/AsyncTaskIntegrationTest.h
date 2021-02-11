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

#ifndef ASYNCTASK_INTEGRATION_TEST_H_
#define ASYNCTASK_INTEGRATION_TEST_H_

#include "asynctask/AsyncTaskMock.h"
#include "asynctask/AsyncTaskThreadPoolMock.h"
#include "asynctask_queue.h"
#include "integration/IntegrationTest.h"

namespace testing {

class AsyncTaskIntegrationTest : public IntegrationTest {
 protected:
  supla_asynctask_queue *queue;
  AsyncTaskThreadPoolMock *pool;
  void WaitForState(AsyncTaskMock *task, async_task_state expected,
                    unsigned int usec);
  void WaitForExec(AsyncTaskThreadPoolMock *pool, unsigned int expected_count,
                   unsigned int usec);

 public:
  virtual void SetUp();
  virtual void TearDown();
  AsyncTaskIntegrationTest();
  virtual ~AsyncTaskIntegrationTest();
};

} /* namespace testing */

#endif /* ASYNCTASK_INTEGRATION_TEST_H_ */
