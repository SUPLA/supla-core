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

#include <ipc/InterruptAndExecuteSceneCommandTest.h>

namespace testing {

void InterruptAndExecuteSceneCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new InterruptAndExecuteSceneCommandMock(socketAdapter);
}

void InterruptAndExecuteSceneCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *InterruptAndExecuteSceneCommandTest::getCommand(
    void) {
  return cmd;
}

TEST_F(InterruptAndExecuteSceneCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(InterruptAndExecuteSceneCommandTest, interruptAndExecuteWithSuccess) {
  EXPECT_CALL(*cmd, interrupt_and_execute(10, 20, IsNull()))
      .WillOnce(Return(true));

  commandProcessingTest("INTERRUPT-AND-EXECUTE-SCENE:10,20\n", "OK:20\n");
}

TEST_F(InterruptAndExecuteSceneCommandTest, interruptAndExecuteWithFail) {
  EXPECT_CALL(*cmd, interrupt_and_execute(10, 20, IsNull()))
      .WillOnce(Return(false));

  commandProcessingTest("INTERRUPT-AND-EXECUTE-SCENE:10,20\n", "UNKNOWN:20\n");
}

TEST_F(InterruptAndExecuteSceneCommandTest, noParams) {
  EXPECT_CALL(*cmd, interrupt_and_execute).Times(0);
  commandProcessingTest("INTERRUPT-AND-EXECUTE-SCENE:\n", "UNKNOWN:0\n");
}

TEST_F(InterruptAndExecuteSceneCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, interrupt_and_execute).Times(0);
  commandProcessingTest("INTERRUPT-AND-EXECUTE-SCENE:0,0\n", "UNKNOWN:0\n");
}

TEST_F(InterruptAndExecuteSceneCommandTest, badParams) {
  EXPECT_CALL(*cmd, interrupt_and_execute).Times(0);
  commandProcessingTest("INTERRUPT-AND-EXECUTE-SCENE:a,10\n", "UNKNOWN:0\n");
}

TEST_F(InterruptAndExecuteSceneCommandTest, googleRequestId) {
  EXPECT_CALL(*cmd, interrupt_and_execute(10, 20, StrEq("RequestID")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "INTERRUPT-AND-EXECUTE-SCENE:10,20,GOOGLE-REQUEST-ID=UmVxdWVzdElE\n",
      "OK:20\n");
}

} /* namespace testing */
