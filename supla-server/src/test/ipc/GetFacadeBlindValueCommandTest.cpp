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

#include "ipc/GetFacadeBlindValueCommandTest.h"

#include "device/value/channel_fb_value.h"

namespace testing {

void GetFacadeBlindValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetFacadeBlindValueCommandMock(socketAdapter);
}

void GetFacadeBlindValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetFacadeBlindValueCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetFacadeBlindValueCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetFacadeBlindValueCommandTest, getFbValueWithoutConfigAndWithSuccess) {
  TDSC_FacadeBlindValue raw_value = {};
  raw_value.position = 70;
  raw_value.tilt = 40;
  raw_value.flags = 0;
  supla_channel_fb_value *value = new supla_channel_fb_value(&raw_value);

  EXPECT_CALL(*cmd, get_channel_value(10, 20, 30)).WillOnce(Return(value));

  commandProcessingTest("GET-FACADE-BLIND-VALUE:10,20,30\n",
                        "VALUE:70,40,72.000000,0\n");
}

TEST_F(GetFacadeBlindValueCommandTest, getFbValueWithConfigAndWithSuccess) {
  TDSC_FacadeBlindValue raw_value = {};
  raw_value.position = 70;
  raw_value.tilt = 40;
  supla_channel_fb_value *value = new supla_channel_fb_value(&raw_value);

  supla_json_config *config = new supla_json_config();
  config->set_user_config("{\"tilt0Angle\":10,\"tilt100Angle\":50}");

  EXPECT_CALL(*cmd, get_channel_value(10, 20, 30)).WillOnce(Return(value));
  EXPECT_CALL(*cmd, get_channel_config(10, 20, 30)).WillOnce(Return(config));

  commandProcessingTest("GET-FACADE-BLIND-VALUE:10,20,30\n",
                        "VALUE:70,40,26.000000,0\n");
}

TEST_F(GetFacadeBlindValueCommandTest, calibrationLost) {
  TDSC_FacadeBlindValue raw_value = {};
  raw_value.position = -1;
  raw_value.tilt = -1;
  raw_value.flags = RS_VALUE_FLAG_CALIBRATION_LOST;
  supla_channel_fb_value *value = new supla_channel_fb_value(&raw_value);

  EXPECT_CALL(*cmd, get_channel_value(10, 20, 30)).WillOnce(Return(value));

  commandProcessingTest("GET-FACADE-BLIND-VALUE:10,20,30\n",
                        "VALUE:-1,-1,-1.800000,4\n");
}

TEST_F(GetFacadeBlindValueCommandTest, getFbValueWithFilure) {
  EXPECT_CALL(*cmd, get_channel_value)
      .WillOnce(Return((supla_channel_fb_value *)nullptr));
  commandProcessingTest("GET-FACADE-BLIND-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetFacadeBlindValueCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_channel_value).Times(0);
  EXPECT_CALL(*cmd, get_channel_config).Times(0);
  commandProcessingTest("GET-FACADE-BLIND-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetFacadeBlindValueCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_channel_value).Times(0);
  EXPECT_CALL(*cmd, get_channel_config).Times(0);
  commandProcessingTest("GET-FACADE-BLIND-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetFacadeBlindValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_channel_value).Times(0);
  EXPECT_CALL(*cmd, get_channel_config).Times(0);
  commandProcessingTest("GET-FACADE-BLIND-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
