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

#include <ipc/OnDeviceDeletedCommandTest.h>

namespace testing {

void OnDeviceDeletedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnDeviceDeletedCommandMock(socketAdapter);
}

void OnDeviceDeletedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnDeviceDeletedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OnDeviceDeletedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnDeviceDeletedCommandTest, deviceDeleted) {
  EXPECT_CALL(*cmd, on_device_deleted(10, 20));

  commandProcessingTest("USER-ON-DEVICE-DELETED:10,20\n", "OK:20\n");
}

TEST_F(OnDeviceDeletedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_device_deleted).Times(0);
  commandProcessingTest("USER-ON-DEVICE-DELETED:\n", "UNKNOWN:0\n");
}

TEST_F(OnDeviceDeletedCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, on_device_deleted).Times(0);
  commandProcessingTest("USER-ON-DEVICE-DELETED:0,10\n", "UNKNOWN:10\n");
}

TEST_F(OnDeviceDeletedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, on_device_deleted).Times(0);
  commandProcessingTest("USER-ON-DEVICE-DELETED:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OnDeviceDeletedCommandTest, badParams) {
  EXPECT_CALL(*cmd, on_device_deleted).Times(0);
  commandProcessingTest("USER-ON-DEVICE-DELETED:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
