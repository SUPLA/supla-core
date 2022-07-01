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

#include "ipc/ClientReconnectCommandTest.h"

namespace testing {

void ClientReconnectCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new ClientReconnectCommandMock(socketAdapter);
}

void ClientReconnectCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *ClientReconnectCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(ClientReconnectCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(ClientReconnectCommandTest, success) {
  EXPECT_CALL(*cmd, reconnect(10, 20)).WillOnce(Return(true));

  commandProcessingTest("CLIENT-RECONNECT:10,20\n", "OK:20\n");
}

TEST_F(ClientReconnectCommandTest, filure) {
  EXPECT_CALL(*cmd, reconnect).WillOnce(Return(false));
  commandProcessingTest("CLIENT-RECONNECT:10,20\n", "USER_OR_CLIENT_UNKNOWN\n");
}

TEST_F(ClientReconnectCommandTest, noParams) {
  EXPECT_CALL(*cmd, reconnect).Times(0);
  commandProcessingTest("CLIENT-RECONNECT:\n", "USER_OR_CLIENT_UNKNOWN\n");
}

TEST_F(ClientReconnectCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, reconnect).Times(0);
  commandProcessingTest("CLIENT-RECONNECT:0,0\n", "USER_OR_CLIENT_UNKNOWN\n");
}

TEST_F(ClientReconnectCommandTest, badParams) {
  EXPECT_CALL(*cmd, reconnect).Times(0);
  commandProcessingTest("CLIENT-RECONNECT:a,10\n", "USER_OR_CLIENT_UNKNOWN\n");
}

} /* namespace testing */
