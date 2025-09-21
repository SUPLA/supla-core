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

#include "OtaPerformUpdateCommandTest.h"

namespace testing {

void OtaPerformUpdateCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OtaPerformUpdateCommandMock(socketAdapter);
}

void OtaPerformUpdateCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OtaPerformUpdateCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OtaPerformUpdateCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OtaPerformUpdateCommandTest, performUpdateWithFailure) {
  EXPECT_CALL(*cmd, perform_update(10, 20)).WillOnce(Return(false));

  commandProcessingTest("OTA-PERFORM-UPDATE:10,20\n", "UNKNOWN:10\n");
}

TEST_F(OtaPerformUpdateCommandTest, performUpdateWithSuccess) {
  EXPECT_CALL(*cmd, perform_update(10, 20)).WillOnce(Return(true));

  commandProcessingTest("OTA-PERFORM-UPDATE:10,20\n", "OK:10\n");
}

TEST_F(OtaPerformUpdateCommandTest, noParams) {
  EXPECT_CALL(*cmd, perform_update).Times(0);
  commandProcessingTest("OTA-PERFORM-UPDATE:\n", "UNKNOWN:0\n");
}

TEST_F(OtaPerformUpdateCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, perform_update).Times(0);
  commandProcessingTest("OTA-PERFORM-UPDATE:10,0\n", "UNKNOWN:10\n");
}

TEST_F(OtaPerformUpdateCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, perform_update).Times(0);
  commandProcessingTest("OTA-PERFORM-UPDATE:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OtaPerformUpdateCommandTest, badParams) {
  EXPECT_CALL(*cmd, perform_update).Times(0);
  commandProcessingTest("OTA-PERFORM-UPDATE:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
