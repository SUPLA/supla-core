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

#include <ipc/BeforeChannelFunctionChangeCommandTest.h>

namespace testing {

void BeforeChannelFunctionChangeCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new BeforeChannelFunctionChangeCommandMock(socketAdapter);
}

void BeforeChannelFunctionChangeCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *BeforeChannelFunctionChangeCommandTest::getCommand(
    void) {
  return cmd;
}

TEST_F(BeforeChannelFunctionChangeCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(BeforeChannelFunctionChangeCommandTest, beforeChange) {
  EXPECT_CALL(*cmd, before_channel_function_change(10, 20));

  commandProcessingTest("USER-BEFORE-CHANNEL-FUNCTION-CHANGE:10,20\n",
                        "OK:20\n");
}

TEST_F(BeforeChannelFunctionChangeCommandTest, noParams) {
  EXPECT_CALL(*cmd, before_channel_function_change).Times(0);
  commandProcessingTest("USER-BEFORE-CHANNEL-FUNCTION-CHANGE:\n",
                        "UNKNOWN:0\n");
}

TEST_F(BeforeChannelFunctionChangeCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, before_channel_function_change).Times(0);
  commandProcessingTest("USER-BEFORE-CHANNEL-FUNCTION-CHANGE:0,10\n",
                        "UNKNOWN:10\n");
}

TEST_F(BeforeChannelFunctionChangeCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, before_channel_function_change).Times(0);
  commandProcessingTest("USER-BEFORE-CHANNEL-FUNCTION-CHANGE:0,0\n",
                        "UNKNOWN:0\n");
}

TEST_F(BeforeChannelFunctionChangeCommandTest, badParams) {
  EXPECT_CALL(*cmd, before_channel_function_change).Times(0);
  commandProcessingTest("USER-BEFORE-CHANNEL-FUNCTION-CHANGE:a,10\n",
                        "UNKNOWN:0\n");
}

} /* namespace testing */
