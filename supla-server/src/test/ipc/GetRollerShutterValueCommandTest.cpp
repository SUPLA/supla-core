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

#include "ipc/GetRollerShutterValueCommandTest.h"

#include "device/value/channel_fb_value.h"

namespace testing {

void GetRollerShutterValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetRollerShutterValueCommandMock(socketAdapter);
}

void GetRollerShutterValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetRollerShutterValueCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetRollerShutterValueCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetRollerShutterValueCommandTest, getRollerShutterValueWithSuccess) {
  TDSC_RollerShutterValue raw_value = {};
  raw_value.position = 70;
  raw_value.flags = 40;
  supla_channel_rs_value *value = new supla_channel_rs_value(&raw_value);

  EXPECT_CALL(*cmd, get_channel_value(10, 20, 30)).WillOnce(Return(value));

  commandProcessingTest("GET-ROLLER-SHUTTER-VALUE:10,20,30\n", "VALUE:70,40\n");
}

TEST_F(GetRollerShutterValueCommandTest, getRollerShutterValueWithFilure) {
  EXPECT_CALL(*cmd, get_channel_value)
      .WillOnce(Return((supla_channel_rs_value *)nullptr));
  commandProcessingTest("GET-ROLLER-SHUTTER-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetRollerShutterValueCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_channel_value).Times(0);
  commandProcessingTest("GET-ROLLER-SHUTTER-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetRollerShutterValueCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_channel_value).Times(0);
  commandProcessingTest("GET-ROLLER-SHUTTER-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetRollerShutterValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_channel_value).Times(0);
  commandProcessingTest("GET-ROLLER-SHUTTER-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
