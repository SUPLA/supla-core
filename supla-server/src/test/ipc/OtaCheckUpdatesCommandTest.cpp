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

#include "OtaCheckUpdatesCommandTest.h"

namespace testing {

void OtaCheckUpdatesCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OtaCheckUpdatesCommandMock(socketAdapter);
}

void OtaCheckUpdatesCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OtaCheckUpdatesCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OtaCheckUpdatesCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OtaCheckUpdatesCommandTest, checkUpdatesWithFailure) {
  EXPECT_CALL(*cmd, check_updates(10, 20)).WillOnce(Return(false));

  commandProcessingTest("OTA-CHECK-UPDATES:10,20\n", "UNKNOWN:10\n");
}

TEST_F(OtaCheckUpdatesCommandTest, checkUpdatesWithSuccess) {
  EXPECT_CALL(*cmd, check_updates(10, 20)).WillOnce(Return(true));

  commandProcessingTest("OTA-CHECK-UPDATES:10,20\n", "OK:10\n");
}

TEST_F(OtaCheckUpdatesCommandTest, noParams) {
  EXPECT_CALL(*cmd, check_updates).Times(0);
  commandProcessingTest("OTA-CHECK-UPDATES:\n", "UNKNOWN:0\n");
}

TEST_F(OtaCheckUpdatesCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, check_updates).Times(0);
  commandProcessingTest("OTA-CHECK-UPDATES:10,0\n", "UNKNOWN:10\n");
}

TEST_F(OtaCheckUpdatesCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, check_updates).Times(0);
  commandProcessingTest("OTA-CHECK-UPDATES:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OtaCheckUpdatesCommandTest, badParams) {
  EXPECT_CALL(*cmd, check_updates).Times(0);
  commandProcessingTest("OTA-CHECK-UPDATES:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
