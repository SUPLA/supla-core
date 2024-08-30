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

#include "ipc/IdentifySubDeviceCommandTest.h"

namespace testing {

void IdentifySubDeviceCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new IdentifySubDeviceCommandMock(socketAdapter);
}

void IdentifySubDeviceCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *IdentifySubDeviceCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(IdentifySubDeviceCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(IdentifySubDeviceCommandTest, IdentifyWithSuccess) {
  EXPECT_CALL(*cmd, identify_subdevice(10, 20, 30)).WillOnce(Return(true));

  commandProcessingTest("IDENTIFY-SUBDEVICE:10,20,30\n", "OK:30\n");
}

TEST_F(IdentifySubDeviceCommandTest, IdentifyWithFilure) {
  EXPECT_CALL(*cmd, identify_subdevice).WillOnce(Return(false));
  commandProcessingTest("IDENTIFY-SUBDEVICE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(IdentifySubDeviceCommandTest, noParams) {
  EXPECT_CALL(*cmd, identify_subdevice).Times(0);
  commandProcessingTest("IDENTIFY-SUBDEVICE:\n", "UNKNOWN:0\n");
}

TEST_F(IdentifySubDeviceCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, identify_subdevice).Times(0);
  commandProcessingTest("IDENTIFY-SUBDEVICE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(IdentifySubDeviceCommandTest, badParams) {
  EXPECT_CALL(*cmd, identify_subdevice).Times(0);
  commandProcessingTest("IDENTIFY-SUBDEVICE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
