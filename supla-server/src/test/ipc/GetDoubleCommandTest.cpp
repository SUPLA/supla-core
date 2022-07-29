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

#include "ipc/GetDoubleCommandTest.h"

namespace testing {

void GetDoubleCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetDoubleCommandMock(socketAdapter);
}

void GetDoubleCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetDoubleCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetDoubleCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetDoubleCommandTest, getDoubleWithSuccess) {
  EXPECT_CALL(*cmd, get_channel_double_value(10, 20, 30, _))
      .WillOnce([](int user_id, int device_id, int channel_id, double *value) {
        *value = 23.12;
        return true;
      });

  commandProcessingTest("GET-DOUBLE-VALUE:10,20,30\n", "VALUE:23.120000\n");
}

TEST_F(GetDoubleCommandTest, getDoubleWithFilure) {
  EXPECT_CALL(*cmd, get_channel_double_value).WillOnce(Return(false));
  commandProcessingTest("GET-DOUBLE-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetDoubleCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_channel_double_value).Times(0);
  commandProcessingTest("GET-DOUBLE-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetDoubleCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_channel_double_value).Times(0);
  commandProcessingTest("GET-DOUBLE-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetDoubleCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_channel_double_value).Times(0);
  commandProcessingTest("GET-DOUBLE-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
