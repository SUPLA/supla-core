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

#include "ActionExecutionIntegrationTest.h"

#include "proto.h"

namespace testing {

ActionExecutionIntegrationTest::ActionExecutionIntegrationTest() {
  memset(&result, 0, sizeof(TSC_ActionExecutionResult));
}

void ActionExecutionIntegrationTest::SetUp() {
  ProperlyRegistered::SetUp();
  memset(&result, 0, sizeof(TSC_ActionExecutionResult));
}

ActionExecutionIntegrationTest::~ActionExecutionIntegrationTest() {}

void ActionExecutionIntegrationTest::onActionExecutionResult(
    TSC_ActionExecutionResult *result) {
  ASSERT_FALSE(result == NULL);
  memcpy(&this->result, result, sizeof(TSC_ActionExecutionResult));
  cancelIteration();
}

TEST_F(ActionExecutionIntegrationTest, channelNotFound) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_execute_action(sclient, ACTION_OPEN, nullptr, nullptr,
                                        ACTION_SUBJECT_TYPE_CHANNEL, 55667788),
            0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(result.ResultCode, SUPLA_RESULTCODE_SUBJECT_NOT_FOUND);
  ASSERT_EQ(result.ActionId, ACTION_OPEN);
  ASSERT_EQ(result.SubjectType, ACTION_SUBJECT_TYPE_CHANNEL);
  ASSERT_EQ(result.SubjectId, 55667788);
}

} /* namespace testing */
