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

#include "ipc/UserReconnectCommandTest.h"

namespace testing {

void UserReconnectCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new UserReconnectCommandMock(socketAdapter);
}

void UserReconnectCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *UserReconnectCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(UserReconnectCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(UserReconnectCommandTest, success) {
  EXPECT_CALL(*cmd, reconnect(10)).WillOnce(Return(true));

  commandProcessingTest("USER-RECONNECT:10\n", "OK:10\n");
}

TEST_F(UserReconnectCommandTest, filure) {
  EXPECT_CALL(*cmd, reconnect).WillOnce(Return(false));
  commandProcessingTest("USER-RECONNECT:10\n", "USER_UNKNOWN:10\n");
}

TEST_F(UserReconnectCommandTest, noParams) {
  EXPECT_CALL(*cmd, reconnect).Times(0);
  commandProcessingTest("USER-RECONNECT:\n", "USER_UNKNOWN:0\n");
}

TEST_F(UserReconnectCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, reconnect).Times(0);
  commandProcessingTest("USER-RECONNECT:0,0,0\n", "USER_UNKNOWN:0\n");
}

TEST_F(UserReconnectCommandTest, badParams) {
  EXPECT_CALL(*cmd, reconnect).Times(0);
  commandProcessingTest("USER-RECONNECT:a,10,c\n", "USER_UNKNOWN:0\n");
}

} /* namespace testing */
