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

#include "ipc/SetCharCommandTest.h"

namespace testing {

void SetCharCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new SetCharCommandMock(socketAdapter);
  user = supla_user::find(10, true);
}

void SetCharCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *SetCharCommandTest::getCommand(void) { return cmd; }

TEST_F(SetCharCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetCharCommandTest, setCharWithSuccess) {
  EXPECT_CALL(*cmd, set_channel_char_value(user, 20, 30, 1, NULL, NULL))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CHAR-VALUE:10,20,30,1\n", "OK:30\n");
}

TEST_F(SetCharCommandTest, setCharWithFilure) {
  EXPECT_CALL(*cmd, set_channel_char_value).WillOnce(Return(false));
  commandProcessingTest("SET-CHAR-VALUE:10,20,30,1\n", "FAIL:30\n");
}

TEST_F(SetCharCommandTest, noParams) {
  EXPECT_CALL(*cmd, set_channel_char_value).Times(0);
  commandProcessingTest("SET-CHAR-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(SetCharCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, set_channel_char_value).Times(0);
  commandProcessingTest("SET-CHAR-VALUE:0,0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(SetCharCommandTest, badParams) {
  EXPECT_CALL(*cmd, set_channel_char_value).Times(0);
  commandProcessingTest("SET-CHAR-VALUE:a,10,c,1\n", "UNKNOWN:0\n");
}

TEST_F(SetCharCommandTest, valueLessThanZero) {
  EXPECT_CALL(*cmd, set_channel_char_value).Times(0);
  commandProcessingTest("SET-CHAR-VALUE:10,20,30,-1\n", "VALUE OUT OF RANGE\n");
}

TEST_F(SetCharCommandTest, max) {
  EXPECT_CALL(*cmd, set_channel_char_value(user, 20, 30, 255, NULL, NULL))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CHAR-VALUE:10,20,30,255\n", "OK:30\n");
}

TEST_F(SetCharCommandTest, valueGreaterThan255) {
  EXPECT_CALL(*cmd, set_channel_char_value).Times(0);
  commandProcessingTest("SET-CHAR-VALUE:10,20,30,256\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetCharCommandTest, alexaCorrelationToken) {
  EXPECT_CALL(*cmd,
              set_channel_char_value(user, 20, 30, 1, StrEq("TokenABCD"), NULL))
      .WillOnce(Return(true));

  commandProcessingTest(
      "SET-CHAR-VALUE:10,20,30,1,ALEXA-CORRELATION-TOKEN=VG9rZW5BQkNE\n",
      "OK:30\n");
}

TEST_F(SetCharCommandTest, googleRequestId) {
  EXPECT_CALL(*cmd,
              set_channel_char_value(user, 20, 30, 1, NULL, StrEq("RequestID")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "SET-CHAR-VALUE:10,20,30,1,GOOGLE-REQUEST-ID=UmVxdWVzdElE\n", "OK:30\n");
}

} /* namespace testing */
