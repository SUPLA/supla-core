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

#include "ipc/IsClientConnectedCommandTest.h"

namespace testing {

void IsClientConnectedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new IsClientConnectedCommandMock(socketAdapter);
}

void IsClientConnectedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *IsClientConnectedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(IsClientConnectedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(IsClientConnectedCommandTest, connected) {
  EXPECT_CALL(*cmd, is_client_online(10, 20)).WillOnce(Return(true));
  commandProcessingTest("IS-CLIENT-CONNECTED:10,20\n", "CONNECTED:20\n");
}

TEST_F(IsClientConnectedCommandTest, disconnected) {
  EXPECT_CALL(*cmd, is_client_online(10, 20)).WillOnce(Return(false));
  commandProcessingTest("IS-CLIENT-CONNECTED:10,20\n", "DISCONNECTED:20\n");
}

TEST_F(IsClientConnectedCommandTest, noParams) {
  EXPECT_CALL(*cmd, is_client_online).Times(0);
  commandProcessingTest("IS-CLIENT-CONNECTED:\n", "DISCONNECTED:0\n");
}

TEST_F(IsClientConnectedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, is_client_online).Times(0);
  commandProcessingTest("IS-CLIENT-CONNECTED:0,0\n", "DISCONNECTED:0\n");
}

TEST_F(IsClientConnectedCommandTest, badParams) {
  EXPECT_CALL(*cmd, is_client_online).Times(0);
  commandProcessingTest("IS-CLIENT-CONNECTED:a,10\n", "DISCONNECTED:0\n");
}

} /* namespace testing */
