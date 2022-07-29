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

#include "ipc/EnterCfgModeCommandTest.h"

namespace testing {

void EnterCfgModeCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new EnterCfgModeCommandMock(socketAdapter);
}

void EnterCfgModeCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *EnterCfgModeCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(EnterCfgModeCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(EnterCfgModeCommandTest, ResetCountersWithSuccess) {
  EXPECT_CALL(*cmd, enter_cfg_mode(10, 30)).WillOnce(Return(true));

  commandProcessingTest("ENTER-CONFIGURATION-MODE:10,30\n", "OK:30\n");
}

TEST_F(EnterCfgModeCommandTest, ResetCountersWithFilure) {
  EXPECT_CALL(*cmd, enter_cfg_mode).WillOnce(Return(false));
  commandProcessingTest("ENTER-CONFIGURATION-MODE:10,30\n", "UNKNOWN:30\n");
}

TEST_F(EnterCfgModeCommandTest, noParams) {
  EXPECT_CALL(*cmd, enter_cfg_mode).Times(0);
  commandProcessingTest("ENTER-CONFIGURATION-MODE:\n", "UNKNOWN:0\n");
}

TEST_F(EnterCfgModeCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, enter_cfg_mode).Times(0);
  commandProcessingTest("ENTER-CONFIGURATION-MODE:0,0\n", "UNKNOWN:0\n");
}

TEST_F(EnterCfgModeCommandTest, badParams) {
  EXPECT_CALL(*cmd, enter_cfg_mode).Times(0);
  commandProcessingTest("ENTER-CONFIGURATION-MODE:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
