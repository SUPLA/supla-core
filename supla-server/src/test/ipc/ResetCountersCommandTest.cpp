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

#include "ipc/ResetCountersCommandTest.h"

namespace testing {

void ResetCountersCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new ResetCountersCommandMock(socketAdapter);
}

void ResetCountersCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *ResetCountersCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(ResetCountersCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(ResetCountersCommandTest, ResetCountersWithSuccess) {
  EXPECT_CALL(*cmd, reset_counters(10, 20, 30)).WillOnce(Return(true));

  commandProcessingTest("RESET-COUNTERS:10,20,30\n", "OK:30\n");
}

TEST_F(ResetCountersCommandTest, ResetCountersWithFilure) {
  EXPECT_CALL(*cmd, reset_counters).WillOnce(Return(false));
  commandProcessingTest("RESET-COUNTERS:10,20,30\n", "FAIL:30\n");
}

TEST_F(ResetCountersCommandTest, noParams) {
  EXPECT_CALL(*cmd, reset_counters).Times(0);
  commandProcessingTest("RESET-COUNTERS:\n", "UNKNOWN:0\n");
}

TEST_F(ResetCountersCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, reset_counters).Times(0);
  commandProcessingTest("RESET-COUNTERS:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(ResetCountersCommandTest, badParams) {
  EXPECT_CALL(*cmd, reset_counters).Times(0);
  commandProcessingTest("RESET-COUNTERS:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
