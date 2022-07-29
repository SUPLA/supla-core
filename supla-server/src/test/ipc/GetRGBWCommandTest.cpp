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

#include "ipc/GetRGBWCommandTest.h"

namespace testing {

void GetRGBWCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetRGBWCommandMock(socketAdapter);
}

void GetRGBWCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetRGBWCommandTest::getCommand(void) { return cmd; }

TEST_F(GetRGBWCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetRGBWCommandTest, getRGBWWithSuccess) {
  EXPECT_CALL(*cmd, get_channel_rgbw_value(10, 20, 30, _, _, _, _))
      .WillOnce([](int user_id, int device_id, int channel_id, int *color,
                   char *color_brightness, char *brightness, char *on_of) {
        *color = 16448250;
        *color_brightness = 54;
        *brightness = 25;
        return true;
      });

  commandProcessingTest("GET-RGBW-VALUE:10,20,30\n", "VALUE:16448250,54,25\n");
}

TEST_F(GetRGBWCommandTest, getRGBWWithFilure) {
  EXPECT_CALL(*cmd, get_channel_rgbw_value).WillOnce(Return(false));
  commandProcessingTest("GET-RGBW-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetRGBWCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_channel_rgbw_value).Times(0);
  commandProcessingTest("GET-RGBW-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetRGBWCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_channel_rgbw_value).Times(0);
  commandProcessingTest("GET-RGBW-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetRGBWCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_channel_rgbw_value).Times(0);
  commandProcessingTest("GET-RGBW-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
