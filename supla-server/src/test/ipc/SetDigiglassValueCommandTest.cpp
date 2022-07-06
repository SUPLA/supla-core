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

#include "ipc/SetDigiglassValueCommandTest.h"

namespace testing {

void SetDigiglassValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new SetDigiglassValueCommandMock(socketAdapter);
}

void SetDigiglassValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *SetDigiglassValueCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(SetDigiglassValueCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetDigiglassValueCommandTest, settDigiglassValueWithSuccess) {
  EXPECT_CALL(*cmd, set_digiglass_value(10, 20, 30, 2, 3))
      .WillOnce(Return(true));

  commandProcessingTest("SET-DIGIGLASS-VALUE:10,20,30,2,3\n", "OK:30\n");
}

TEST_F(SetDigiglassValueCommandTest, setDigiglassValueWithFilure) {
  EXPECT_CALL(*cmd, set_digiglass_value).WillOnce(Return(false));
  commandProcessingTest("SET-DIGIGLASS-VALUE:10,20,30,2,3\n", "FAIL:30\n");
}

TEST_F(SetDigiglassValueCommandTest, noParams) {
  EXPECT_CALL(*cmd, set_digiglass_value).Times(0);
  commandProcessingTest("SET-DIGIGLASS-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(SetDigiglassValueCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, set_digiglass_value).Times(0);
  commandProcessingTest("SET-DIGIGLASS-VALUE:0,0,0,0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(SetDigiglassValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, set_digiglass_value).Times(0);
  commandProcessingTest("SET-DIGIGLASS-VALUE:a,10,c,1,2,3\n", "UNKNOWN:0\n");
}

} /* namespace testing */
