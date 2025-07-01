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

#include "OnChannelAddedCommandTest.h"

namespace testing {

void OnChannelAddedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnChannelAddedCommandMock(socketAdapter);
}

void OnChannelAddedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnChannelAddedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OnChannelAddedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnChannelAddedCommandTest, channelAdded) {
  EXPECT_CALL(*cmd, on_channel_added(10, 20, 30));

  commandProcessingTest("USER-ON-CHANNEL-ADDED:10,20,30\n", "OK:10\n");
}

TEST_F(OnChannelAddedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_channel_added).Times(0);
  commandProcessingTest("USER-ON-CHANNEL-ADDED:\n", "UNKNOWN:0\n");
}

TEST_F(OnChannelAddedCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, on_channel_added).Times(0);
  commandProcessingTest("USER-ON-CHANNEL-ADDED:10,20,0\n", "UNKNOWN:0\n");
}

TEST_F(OnChannelAddedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, on_channel_added).Times(0);
  commandProcessingTest("USER-ON-CHANNEL-ADDED:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(OnChannelAddedCommandTest, badParams) {
  EXPECT_CALL(*cmd, on_channel_added).Times(0);
  commandProcessingTest("USER-ON-CHANNEL-ADDED:a,10,30\n", "UNKNOWN:0\n");
}

} /* namespace testing */
