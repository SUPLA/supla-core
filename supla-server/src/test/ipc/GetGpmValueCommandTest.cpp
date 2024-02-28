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

#include "GetGpmValueCommandTest.h"

#include <math.h>

namespace testing {

void GetGpmValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetGpmValueCommandMock(socketAdapter);
}

void GetGpmValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetGpmValueCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetGpmValueCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetGpmValueCommandTest, getGpmValueWithSuccess) {
  EXPECT_CALL(*cmd, get_gpm_value(10, 20, 30, NotNull()))
      .WillOnce([](int user_id, int device_id, int channel_id, double *result) {
        *result = 123.456;
        return true;
      });

  commandProcessingTest("GET-GPM-VALUE:10,20,30\n", "VALUE:123.456000\n");
}

TEST_F(GetGpmValueCommandTest, getGpmValueWithNanResult) {
  EXPECT_CALL(*cmd, get_gpm_value(10, 20, 30, NotNull()))
      .WillOnce([](int user_id, int device_id, int channel_id, double *result) {
        *result = NAN;
        return true;
      });

  commandProcessingTest("GET-GPM-VALUE:10,20,30\n", "VALUE:nan\n");
}

TEST_F(GetGpmValueCommandTest, getGpmValueWithFilure) {
  EXPECT_CALL(*cmd, get_gpm_value).WillOnce(Return(false));
  commandProcessingTest("GET-GPM-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetGpmValueCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_gpm_value).Times(0);
  commandProcessingTest("GET-GPM-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetGpmValueCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_gpm_value).Times(0);
  commandProcessingTest("GET-GPM-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetGpmValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_gpm_value).Times(0);
  commandProcessingTest("GET-GPM-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
