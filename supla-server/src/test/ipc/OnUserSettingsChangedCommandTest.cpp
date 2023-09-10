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

#include "OnUserSettingsChangedCommandTest.h"

namespace testing {

void OnUserSettingsChangedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnUserSettingsChangedCommandMock(socketAdapter);
}

void OnUserSettingsChangedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnUserSettingsChangedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OnUserSettingsChangedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnUserSettingsChangedCommandTest, deviceSettingsChanged) {
  EXPECT_CALL(*cmd, on_user_settings_changed(10));

  commandProcessingTest("USER-ON-SETTINGS-CHANGED:10\n", "OK:10\n");
}

TEST_F(OnUserSettingsChangedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_user_settings_changed).Times(0);
  commandProcessingTest("USER-ON-SETTINGS-CHANGED:\n", "UNKNOWN:0\n");
}

TEST_F(OnUserSettingsChangedCommandTest, paramWithZero) {
  EXPECT_CALL(*cmd, on_user_settings_changed).Times(0);
  commandProcessingTest("USER-ON-SETTINGS-CHANGED:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OnUserSettingsChangedCommandTest, badParams) {
  EXPECT_CALL(*cmd, on_user_settings_changed).Times(0);
  commandProcessingTest("USER-ON-SETTINGS-CHANGED:a\n", "UNKNOWN:0\n");
}

} /* namespace testing */
