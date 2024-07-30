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

#include "ipc/RestartSubDeviceCommandTest.h"

namespace testing {

void RestartSubDeviceCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new RestartSubDeviceCommandMock(socketAdapter);
}

void RestartSubDeviceCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *RestartSubDeviceCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(RestartSubDeviceCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(RestartSubDeviceCommandTest, RestartWithSuccess) {
  EXPECT_CALL(*cmd, restart_subdevice(10, 20, 30)).WillOnce(Return(true));

  commandProcessingTest("RESTART-SUBDEVICE:10,20,30\n", "OK:30\n");
}

TEST_F(RestartSubDeviceCommandTest, RestartWithFilure) {
  EXPECT_CALL(*cmd, restart_subdevice).WillOnce(Return(false));
  commandProcessingTest("RESTART-SUBDEVICE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(RestartSubDeviceCommandTest, noParams) {
  EXPECT_CALL(*cmd, restart_subdevice).Times(0);
  commandProcessingTest("RESTART-SUBDEVICE:\n", "UNKNOWN:0\n");
}

TEST_F(RestartSubDeviceCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, restart_subdevice).Times(0);
  commandProcessingTest("RESTART-SUBDEVICE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(RestartSubDeviceCommandTest, badParams) {
  EXPECT_CALL(*cmd, restart_subdevice).Times(0);
  commandProcessingTest("RESTART-SUBDEVICE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
