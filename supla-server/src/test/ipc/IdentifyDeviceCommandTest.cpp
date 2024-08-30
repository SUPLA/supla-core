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

#include "ipc/IdentifyDeviceCommandTest.h"

namespace testing {

void IdentifyDeviceCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new IdentifyDeviceCommandMock(socketAdapter);
}

void IdentifyDeviceCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *IdentifyDeviceCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(IdentifyDeviceCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(IdentifyDeviceCommandTest, IdentifyWithSuccess) {
  EXPECT_CALL(*cmd, identify_device(10, 20)).WillOnce(Return(true));

  commandProcessingTest("IDENTIFY-DEVICE:10,20\n", "OK:20\n");
}

TEST_F(IdentifyDeviceCommandTest, IdentifyWithFilure) {
  EXPECT_CALL(*cmd, identify_device).WillOnce(Return(false));
  commandProcessingTest("IDENTIFY-DEVICE:10,20\n", "UNKNOWN:10\n");
}

TEST_F(IdentifyDeviceCommandTest, noParams) {
  EXPECT_CALL(*cmd, identify_device).Times(0);
  commandProcessingTest("IDENTIFY-DEVICE:\n", "UNKNOWN:0\n");
}

TEST_F(IdentifyDeviceCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, identify_device).Times(0);
  commandProcessingTest("IDENTIFY-DEVICE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(IdentifyDeviceCommandTest, badParams) {
  EXPECT_CALL(*cmd, identify_device).Times(0);
  commandProcessingTest("IDENTIFY-DEVICE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
