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

#include "OnDeviceConfigChangedCommandTest.h"

namespace testing {

void OnDeviceConfigChangedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnDeviceConfigChangedCommandMock(socketAdapter);
}

void OnDeviceConfigChangedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnDeviceConfigChangedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OnDeviceConfigChangedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnDeviceConfigChangedCommandTest, deviceSettingsChanged) {
  EXPECT_CALL(*cmd, on_device_config_changed(10, 20));

  commandProcessingTest("USER-ON-DEVICE-CONFIG-CHANGED:10,20\n", "OK:10\n");
}

TEST_F(OnDeviceConfigChangedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_device_config_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-CONFIG-CHANGED:\n", "UNKNOWN:0\n");
}

TEST_F(OnDeviceConfigChangedCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, on_device_config_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-CONFIG-CHANGED:10,0\n", "UNKNOWN:10\n");
}

TEST_F(OnDeviceConfigChangedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, on_device_config_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-CONFIG-CHANGED:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OnDeviceConfigChangedCommandTest, badParams) {
  EXPECT_CALL(*cmd, on_device_config_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-CONFIG-CHANGED:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
