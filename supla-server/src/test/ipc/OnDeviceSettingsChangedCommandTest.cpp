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

#include <ipc/OnDeviceSettingsChangedCommandTest.h>

namespace testing {

void OnDeviceSettingsChangedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnDeviceSettingsChangedCommandMock(socketAdapter);
}

void OnDeviceSettingsChangedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnDeviceSettingsChangedCommandTest::getCommand(
    void) {
  return cmd;
}

TEST_F(OnDeviceSettingsChangedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnDeviceSettingsChangedCommandTest, deviceSettingsChanged) {
  EXPECT_CALL(*cmd, on_device_settings_changed(10, 20));

  commandProcessingTest("USER-ON-DEVICE-SETTINGS-CHANGED:10,20\n", "OK:20\n");
}

TEST_F(OnDeviceSettingsChangedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_device_settings_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-SETTINGS-CHANGED:\n", "UNKNOWN:0\n");
}

TEST_F(OnDeviceSettingsChangedCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, on_device_settings_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-SETTINGS-CHANGED:0,10\n",
                        "UNKNOWN:10\n");
}

TEST_F(OnDeviceSettingsChangedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, on_device_settings_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-SETTINGS-CHANGED:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OnDeviceSettingsChangedCommandTest, badParams) {
  EXPECT_CALL(*cmd, on_device_settings_changed).Times(0);
  commandProcessingTest("USER-ON-DEVICE-SETTINGS-CHANGED:a,10\n",
                        "UNKNOWN:0\n");
}

} /* namespace testing */
