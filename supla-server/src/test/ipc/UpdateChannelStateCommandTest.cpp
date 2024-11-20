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

#include "ipc/UpdateChannelStateCommandTest.h"

namespace testing {

void UpdateChannelStateCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new UpdateChannelStateCommandMock(socketAdapter);
}

void UpdateChannelStateCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *UpdateChannelStateCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(UpdateChannelStateCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(UpdateChannelStateCommandTest, UpdateChannelStateWithSuccess) {
  EXPECT_CALL(*cmd, update_state(10, 20, 30)).WillOnce(Return(true));

  commandProcessingTest("UPDATE-CHANNEL-STATE:10,20,30\n", "OK:30\n");
}

TEST_F(UpdateChannelStateCommandTest, UpdateChannelStateWithFilure) {
  EXPECT_CALL(*cmd, update_state).WillOnce(Return(false));
  commandProcessingTest("UPDATE-CHANNEL-STATE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(UpdateChannelStateCommandTest, noParams) {
  EXPECT_CALL(*cmd, update_state).Times(0);
  commandProcessingTest("UPDATE-CHANNEL-STATE:\n", "UNKNOWN:0\n");
}

TEST_F(UpdateChannelStateCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, update_state).Times(0);
  commandProcessingTest("UPDATE-CHANNEL-STATE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(UpdateChannelStateCommandTest, badParams) {
  EXPECT_CALL(*cmd, update_state).Times(0);
  commandProcessingTest("UPDATE-CHANNEL-STATE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
