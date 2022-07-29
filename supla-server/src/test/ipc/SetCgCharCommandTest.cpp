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

#include "ipc/SetCgCharCommandTest.h"

namespace testing {

void SetCgCharCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new SetCgCharCommandMock(socketAdapter);
  user = supla_user::find(10, true);
}

void SetCgCharCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *SetCgCharCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(SetCgCharCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetCgCharCommandTest, setCgCharWithSuccess) {
  EXPECT_CALL(*cmd, set_cg_char_value(user, 30, 1)).WillOnce(Return(true));

  commandProcessingTest("SET-CG-CHAR-VALUE:10,30,1\n", "OK:30\n");
}

TEST_F(SetCgCharCommandTest, setCgCharWithFilure) {
  EXPECT_CALL(*cmd, set_cg_char_value).WillOnce(Return(false));
  commandProcessingTest("SET-CG-CHAR-VALUE:10,30,1\n", "FAIL:30\n");
}

TEST_F(SetCgCharCommandTest, noParams) {
  EXPECT_CALL(*cmd, set_cg_char_value).Times(0);
  commandProcessingTest("SET-CG-CHAR-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(SetCgCharCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, set_cg_char_value).Times(0);
  commandProcessingTest("SET-CG-CHAR-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(SetCgCharCommandTest, badParams) {
  EXPECT_CALL(*cmd, set_cg_char_value).Times(0);
  commandProcessingTest("SET-CG-CHAR-VALUE:a,10,1\n", "UNKNOWN:0\n");
}

TEST_F(SetCgCharCommandTest, valueLessThanZero) {
  EXPECT_CALL(*cmd, set_cg_char_value).Times(0);
  commandProcessingTest("SET-CG-CHAR-VALUE:10,30,-1\n", "VALUE OUT OF RANGE\n");
}

TEST_F(SetCgCharCommandTest, max) {
  EXPECT_CALL(*cmd, set_cg_char_value(user, 30, 255)).WillOnce(Return(true));

  commandProcessingTest("SET-CG-CHAR-VALUE:10,30,255\n", "OK:30\n");
}

TEST_F(SetCgCharCommandTest, valueGreaterThan255) {
  EXPECT_CALL(*cmd, set_cg_char_value).Times(0);
  commandProcessingTest("SET-CG-CHAR-VALUE:10,30,256\n",
                        "VALUE OUT OF RANGE\n");
}

} /* namespace testing */
