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

#include "SetRGBWCommandTest.h"

#include "gmock/gmock.h"

namespace testing {

void SetRGBWCommandTest::SetUp() {
  IpcCommandTest::SetUp();

  supla_user::find(10, true);

  cmd = nullptr;
  setCmd =
      new supla_set_rgbw_command(socketAdapter, &actionExecutor, false, false);
  setRandCmd =
      new supla_set_rgbw_command(socketAdapter, &actionExecutor, true, false);
  cgSetCmd =
      new supla_set_rgbw_command(socketAdapter, &actionExecutor, false, true);
  cgSetRandCmd =
      new supla_set_rgbw_command(socketAdapter, &actionExecutor, true, true);
}

void SetRGBWCommandTest::TearDown() {
  IpcCommandTest::TearDown();

  delete setCmd;
  delete setRandCmd;
  delete cgSetCmd;
  delete cgSetRandCmd;
}

supla_abstract_ipc_command *SetRGBWCommandTest::getCommand(void) { return cmd; }

TEST_F(SetRGBWCommandTest, noData_set) {
  cmd = setCmd;
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetRGBWCommandTest, noData_setRand) {
  cmd = setRandCmd;
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetRGBWCommandTest, noData_cgSet) {
  cmd = cgSetCmd;
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetRGBWCommandTest, noData_cgSetRand) {
  cmd = cgSetRandCmd;
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetRGBWCommandTest, setRGBW_incorrectNumberOfParams) {
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,1,2,4,3,5\n",
                        "INCORRECT NUMBERS OF PARAMETERS\n");
}

TEST_F(SetRGBWCommandTest, cgSetRGBW_incorrectNumberOfParams) {
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,1,2,4,3,5\n",
                        "INCORRECT NUMBERS OF PARAMETERS\n");
}

TEST_F(SetRGBWCommandTest, setRandomColor_incorrectNumberOfParams) {
  cmd = setRandCmd;
  commandProcessingTest("SET-RAND-RGBW-VALUE:10,0,30,2,4,3,5\n",
                        "INCORRECT NUMBERS OF PARAMETERS\n");
}

TEST_F(SetRGBWCommandTest, cgSetRandomColor_incorrectNumberOfParams) {
  cmd = cgSetRandCmd;
  commandProcessingTest("SET-CG-RAND-RGBW-VALUE:10,30,2,4,3,5\n",
                        "INCORRECT NUMBERS OF PARAMETERS\n");
}

TEST_F(SetRGBWCommandTest, setRGBW) {
  EXPECT_CALL(actionExecutor,
              set_rgbw(Pointee(Eq(1)), Pointee(Eq(2)), Pointee(Eq(4)),
                       Pointee(Eq(3)), Pointee(Eq(5)), Pointee(Eq(6))))
      .Times(1);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,1,2,4,3,5,6\n", "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgSetRGBW) {
  EXPECT_CALL(actionExecutor,
              set_rgbw(Pointee(Eq(1)), Pointee(Eq(2)), Pointee(Eq(4)),
                       Pointee(Eq(3)), Pointee(Eq(5)), Pointee(Eq(6))))
      .Times(1);
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,1,2,4,3,5,6\n", "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, setRandomColor) {
  EXPECT_CALL(actionExecutor,
              set_rgbw(NotNull(), Pointee(Eq(2)), Pointee(Eq(4)),
                       Pointee(Eq(3)), Pointee(Eq(5)), Pointee(Eq(6))))
      .Times(1);
  cmd = setRandCmd;
  commandProcessingTest("SET-RAND-RGBW-VALUE:10,0,30,2,4,3,5,6\n", "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgSetRandomColor) {
  EXPECT_CALL(actionExecutor,
              set_rgbw(NotNull(), Pointee(Eq(2)), Pointee(Eq(4)),
                       Pointee(Eq(3)), Pointee(Eq(5)), Pointee(Eq(6))))
      .Times(1);
  cmd = cgSetRandCmd;
  commandProcessingTest("SET-CG-RAND-RGBW-VALUE:10,30,2,4,3,5,6\n", "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, onlyColor) {
  EXPECT_CALL(actionExecutor, set_rgbw(Pointee(Eq(1)), IsNull(), IsNull(),
                                       IsNull(), IsNull(), IsNull()))
      .Times(1);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,1,-1,-1,-1,-1,-1\n", "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgOnlyColor) {
  EXPECT_CALL(actionExecutor, set_rgbw(Pointee(Eq(1)), IsNull(), IsNull(),
                                       IsNull(), IsNull(), IsNull()))
      .Times(1);
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,1,-1,-1,-1,-1,-1\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, onlyColorBrightness) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), Pointee(Eq(50)), IsNull(),
                                       IsNull(), IsNull(), IsNull()))
      .Times(1);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,50,-1,-1,-1,-1\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgOnlyColorBrightness) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), Pointee(Eq(60)), IsNull(),
                                       IsNull(), IsNull(), IsNull()))
      .Times(1);
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,-1,60,-1,-1,-1,-1\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, onlyBrightness) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), Pointee(Eq(20)),
                                       IsNull(), IsNull(), IsNull()))
      .Times(1);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,20,-1,-1,-1\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgOnlyBrightness) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), Pointee(Eq(100)),
                                       IsNull(), IsNull(), IsNull()))
      .Times(1);
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,-1,-1,100,-1,-1,-1\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, onlyOnOff) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), IsNull(),
                                       Pointee(Eq(1)), IsNull(), IsNull()))
      .Times(1);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,1,-1,-1\n", "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgOnlyOnOff) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), IsNull(),
                                       Pointee(Eq(2)), IsNull(), IsNull()))
      .Times(1);
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,-1,-1,-1,2,-1,-1\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, onlyCommand) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), IsNull(), IsNull(),
                                       Pointee(Eq(5)), IsNull()))
      .Times(1);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-1,5,-1\n", "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgOnlyCommand) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), IsNull(), IsNull(),
                                       Pointee(Eq(1)), IsNull()))
      .Times(1);
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,-1,-1,-1,-1,1,-1\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, onlyWhiteTemperature) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), IsNull(), IsNull(),
                                       IsNull(), Pointee(Eq(25))))
      .Times(1);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-1,-1,25\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_channel_id(), 30);
}

TEST_F(SetRGBWCommandTest, cgOnlyWhiteTemperature) {
  EXPECT_CALL(actionExecutor, set_rgbw(IsNull(), IsNull(), IsNull(), IsNull(),
                                       IsNull(), Pointee(Eq(12))))
      .Times(1);
  cmd = cgSetCmd;
  commandProcessingTest("SET-CG-RGBW-VALUE:10,30,-1,-1,-1,-1,-1,12\n",
                        "OK:30\n");

  EXPECT_EQ(actionExecutor.get_user_id(), 10);
  EXPECT_EQ(actionExecutor.get_group_id(), 30);
}

TEST_F(SetRGBWCommandTest, alexaCorrelationToken) {
  cmd = setCmd;
  EXPECT_CALL(actionExecutor,
              on_correlation_token(StrEq("TokenABCD"), IsNull()))
      .Times(1);

  commandProcessingTest(
      "SET-RGBW-VALUE:10,0,30,1,2,4,3,5,6,ALEXA-CORRELATION-TOKEN="
      "VG9rZW5BQkNE\n",
      "OK:30\n");
}

TEST_F(SetRGBWCommandTest, googleRequestId) {
  cmd = setCmd;
  EXPECT_CALL(actionExecutor,
              on_correlation_token(IsNull(), StrEq("RequestID")))
      .Times(1);

  commandProcessingTest(
      "SET-RGBW-VALUE:10,0,30,1,2,4,3,5,6,GOOGLE-REQUEST-ID=UmVxdWVzdElE\n",
      "OK:30\n");
}

TEST_F(SetRGBWCommandTest, colorBrightnessOutOfRange) {
  EXPECT_CALL(actionExecutor, set_rgbw).Times(0);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-2,-1,-1,-1,-1\n",
                        "VALUE OUT OF RANGE\n");
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,101,-1,-1,-1,-1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, brightnessOutOfRange) {
  EXPECT_CALL(actionExecutor, set_rgbw).Times(0);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-2,-1,-1,-1\n",
                        "VALUE OUT OF RANGE\n");
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-101,-1,-1,-1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, onOffOutOfRange) {
  EXPECT_CALL(actionExecutor, set_rgbw).Times(0);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-2,-1,-1\n",
                        "VALUE OUT OF RANGE\n");
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-4,-1,-1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, commandOutOfRange) {
  EXPECT_CALL(actionExecutor, set_rgbw).Times(0);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-1,-2,-1\n",
                        "VALUE OUT OF RANGE\n");
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-1,20,-1\n",
                        "VALUE OUT OF RANGE\n");
}

TEST_F(SetRGBWCommandTest, whiteTemperatureOutOfRange) {
  EXPECT_CALL(actionExecutor, set_rgbw).Times(0);
  cmd = setCmd;
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-1,-1,-2\n",
                        "VALUE OUT OF RANGE\n");
  commandProcessingTest("SET-RGBW-VALUE:10,0,30,-1,-1,-1,-1,-1,101\n",
                        "VALUE OUT OF RANGE\n");
}

} /* namespace testing */
