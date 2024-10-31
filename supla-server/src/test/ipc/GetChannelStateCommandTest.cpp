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

#include "ipc/GetChannelStateCommandTest.h"

namespace testing {

void GetChannelStateCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetChannelStateCommandMock(socketAdapter);
}

void GetChannelStateCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetChannelStateCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetChannelStateCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetChannelStateCommandTest, getChannelStateWithSuccess) {}

TEST_F(GetChannelStateCommandTest, getChannelStateWithFilure) {
  EXPECT_CALL(*cmd, get_channel_state).WillOnce(Return(false));
  commandProcessingTest("GET-CHANNEL-STATE:10,20,30,0,0\n",
                        "NOT-AVAILABLE:30\n");
}

TEST_F(GetChannelStateCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_channel_state).Times(0);
  commandProcessingTest("GET-CHANNEL-STATE:\n", "UNKNOWN:0\n");
}

TEST_F(GetChannelStateCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_channel_state).Times(0);
  commandProcessingTest("GET-CHANNEL-STATE:0,0,0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetChannelStateCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_channel_state).Times(0);
  commandProcessingTest("GET-CHANNEL-STATE:a,10,c,0,0\n", "UNKNOWN:0\n");
}

} /* namespace testing */
