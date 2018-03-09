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
#include "ActionTest.h"
#include "../action.h"
#include "gtest/gtest.h"  // NOLINT

#define MIN_RETRY_TIME 5
#define MIN_CHECK_TIME 1

namespace {

class ActionTest : public ::testing::Test {
 protected:
};

TEST_F(ActionTest, time) {
  s_worker *worker = new s_worker(NULL);
  ASSERT_FALSE(worker == NULL);

  for (auto &&factory : AbstractActionFactory::factories) {
    s_worker_action *action = factory->create(worker);
    ASSERT_FALSE(action == NULL);
    ASSERT_GE(action->waiting_time_to_retry(), MIN_RETRY_TIME);
    ASSERT_GE(action->waiting_time_to_check(), MIN_CHECK_TIME);

    int diff =
        action->waiting_time_to_retry() - action->waiting_time_to_check();
    ASSERT_GT(diff, 0);

    delete action;
  }

  delete worker;
}

}  // namespace
