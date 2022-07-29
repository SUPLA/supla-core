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

#include "ipc/IsChannelConnectedCommandTest.h"

namespace testing {

void IsChannelConnectedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new IsChannelConnectedCommandMock(socketAdapter);
}

void IsChannelConnectedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *IsChannelConnectedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(IsChannelConnectedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(IsChannelConnectedCommandTest, connected) {
  EXPECT_CALL(*cmd, is_channel_online(10, 20, 30)).WillOnce(Return(true));
  commandProcessingTest("IS-CHANNEL-CONNECTED:10,20,30\n", "CONNECTED:30\n");
}

TEST_F(IsChannelConnectedCommandTest, disconnected) {
  EXPECT_CALL(*cmd, is_channel_online(10, 20, 30)).WillOnce(Return(false));
  commandProcessingTest("IS-CHANNEL-CONNECTED:10,20,30\n", "DISCONNECTED:30\n");
}

TEST_F(IsChannelConnectedCommandTest, noParams) {
  EXPECT_CALL(*cmd, is_channel_online).Times(0);
  commandProcessingTest("IS-CHANNEL-CONNECTED:\n", "DISCONNECTED:0\n");
}

TEST_F(IsChannelConnectedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, is_channel_online).Times(0);
  commandProcessingTest("IS-CHANNEL-CONNECTED:0,0,0\n", "DISCONNECTED:0\n");
}

TEST_F(IsChannelConnectedCommandTest, badParams) {
  EXPECT_CALL(*cmd, is_channel_online).Times(0);
  commandProcessingTest("IS-CHANNEL-CONNECTED:a,10,b\n", "DISCONNECTED:0\n");
}

} /* namespace testing */
