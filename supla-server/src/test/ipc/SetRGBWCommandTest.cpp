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

#include "ipc/SetRGBWCommandTest.h"

namespace testing {

void SetRGBWCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new SetRGBWCommandMock(socketAdapter, false);
  user = supla_user::find(10, true);
}

void SetRGBWCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *SetRGBWCommandTest::getCommand(void) { return cmd; }

TEST_F(SetRGBWCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetRGBWCommandTest, setRGBWWithSuccess) {
  EXPECT_CALL(*cmd,
              set_channel_rgbw_value(user, 20, 30, 4, 3, 2, 1, NULL, NULL))
      .WillOnce(Return(true));

  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,3,2,1\n", "OK:30\n");
}

TEST_F(SetRGBWCommandTest, setRGBWWithFilure) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).WillOnce(Return(false));
  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,3,2,1\n", "FAIL:30\n");
}

TEST_F(SetRGBWCommandTest, noParams) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).Times(0);
  commandProcessingTest("SET-RGBW-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(SetRGBWCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).Times(0);
  commandProcessingTest("SET-RGBW-VALUE:0,0,0,0,0,0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(SetRGBWCommandTest, badParams) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).Times(0);
  commandProcessingTest("SET-RGBW-VALUE:a,10,c,1,4,3,2,1\n", "UNKNOWN:0\n");
}

TEST_F(SetRGBWCommandTest, colorBrightnessMin) {
  EXPECT_CALL(*cmd,
              set_channel_rgbw_value(user, 20, 30, 4, 0, 2, 1, NULL, NULL))
      .WillOnce(Return(true));

  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,0,2,1\n", "OK:30\n");
}

TEST_F(SetRGBWCommandTest, colorBrightnessMax) {
  EXPECT_CALL(*cmd,
              set_channel_rgbw_value(user, 20, 30, 4, 100, 2, 1, NULL, NULL))
      .WillOnce(Return(true));

  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,100,2,1\n", "OK:30\n");
}

TEST_F(SetRGBWCommandTest, colorBrightnessTooLow) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).Times(0);
  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,-1,2,1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, colorBrightnessTooHigh) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).Times(0);
  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,101,2,1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, brightnessMin) {
  EXPECT_CALL(*cmd,
              set_channel_rgbw_value(user, 20, 30, 4, 3, 0, 1, NULL, NULL))
      .WillOnce(Return(true));

  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,3,0,1\n", "OK:30\n");
}

TEST_F(SetRGBWCommandTest, brightnessMax) {
  EXPECT_CALL(*cmd,
              set_channel_rgbw_value(user, 20, 30, 4, 3, 100, 1, NULL, NULL))
      .WillOnce(Return(true));

  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,3,100,1\n", "OK:30\n");
}

TEST_F(SetRGBWCommandTest, brightnessTooLow) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).Times(0);
  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,3,-1,1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, brightnessTooHigh) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value).Times(0);
  commandProcessingTest("SET-RGBW-VALUE:10,20,30,4,3,101,1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, alexaCorrelationToken) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value(user, 20, 30, 4, 3, 2, 1,
                                           StrEq("TokenABCD"), NULL))
      .WillOnce(Return(true));

  commandProcessingTest(
      "SET-RGBW-VALUE:10,20,30,4,3,2,1,ALEXA-CORRELATION-TOKEN=VG9rZW5BQkNE\n",
      "OK:30\n");
}

TEST_F(SetRGBWCommandTest, googleRequestId) {
  EXPECT_CALL(*cmd, set_channel_rgbw_value(user, 20, 30, 4, 3, 2, 1, NULL,
                                           StrEq("RequestID")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "SET-RGBW-VALUE:10,20,30,4,3,2,1,GOOGLE-REQUEST-ID=UmVxdWVzdElE\n",
      "OK:30\n");
}

} /* namespace testing */
