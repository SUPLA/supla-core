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

#include "ipc/SetCgRGBWCommandTest.h"

namespace testing {

void SetCgRGBWCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new SetCgRGBWCommandMock(socketAdapter, false);
  user = supla_user::find(10, true);
}

void SetCgRGBWCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *SetCgRGBWCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(SetCgRGBWCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetCgRGBWCommandTest, setRGBWWithSuccess) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value(user, 30, 4, 3, 2, 1))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,3,2,1\n", "OK:30\n");
}

TEST_F(SetCgRGBWCommandTest, randomColor) {
  delete cmd;
  cmd = new SetCgRGBWCommandMock(socketAdapter, true);

  int color1 = 0;
  int color2 = 0;

  EXPECT_CALL(*cmd, set_cg_rgbw_value(user, 30, Gt(0), 3, 2, 1))
      .WillOnce(DoAll(SaveArg<3>(&color1), Return(true)));

  char cmdString[] = "SET-CG-RAND-RGBW-VALUE:10,30,3,2,1\n";
  char expectedResult[] = "OK:30\n";

  commandProcessingTest(cmdString, expectedResult);

  delete cmd;
  cmd = new SetCgRGBWCommandMock(socketAdapter, true);

  EXPECT_CALL(*cmd, set_cg_rgbw_value(user, 30, Gt(0), 3, 2, 1))
      .WillOnce(DoAll(SaveArg<3>(&color2), Return(true)));

  commandProcessingTest(cmdString, expectedResult);

  EXPECT_NE(color1, color2);
}

TEST_F(SetCgRGBWCommandTest, setRGBWWithFilure) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).WillOnce(Return(false));
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,3,2,1\n", "FAIL:30\n");
}

TEST_F(SetCgRGBWCommandTest, noParams) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).Times(0);
  commandProcessingTest("SET-CG-RGBW-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(SetCgRGBWCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).Times(0);
  commandProcessingTest("SET-CG-RGBW-VALUE:0,0,0,0,0,0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(SetCgRGBWCommandTest, badParams) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).Times(0);
  commandProcessingTest("SET-CG-RGBW-VALUE:a,10,c,1,4,3,2,1\n", "UNKNOWN:0\n");
}

TEST_F(SetCgRGBWCommandTest, colorBrightnessMin) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value(user, 30, 4, 0, 2, 1))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,0,2,1\n", "OK:30\n");
}

TEST_F(SetCgRGBWCommandTest, colorBrightnessMax) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value(user, 30, 4, 100, 2, 1))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,100,2,1\n", "OK:30\n");
}

TEST_F(SetCgRGBWCommandTest, colorBrightnessTooLow) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).Times(0);
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,-1,2,1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetCgRGBWCommandTest, colorBrightnessTooHigh) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).Times(0);
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,101,2,1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetCgRGBWCommandTest, brightnessMin) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value(user, 30, 4, 3, 0, 1))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,3,0,1\n", "OK:30\n");
}

TEST_F(SetCgRGBWCommandTest, brightnessMax) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value(user, 30, 4, 3, 100, 1))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,3,100,1\n", "OK:30\n");
}

TEST_F(SetCgRGBWCommandTest, brightnessTooLow) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).Times(0);
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,3,-1,1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetCgRGBWCommandTest, brightnessTooHigh) {
  EXPECT_CALL(*cmd, set_cg_rgbw_value).Times(0);
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,4,3,101,1\n",
                        "VALUE OUT OF RANGE\n");
}
} /* namespace testing */
