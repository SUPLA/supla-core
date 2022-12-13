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

#include <ipc/ExecuteSceneCommandTest.h>

namespace testing {

void ExecuteSceneCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new ExecuteSceneCommandMock(socketAdapter);
}

void ExecuteSceneCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *ExecuteSceneCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(ExecuteSceneCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(ExecuteSceneCommandTest, executeWithSuccess) {
  EXPECT_CALL(*cmd, execute(10, 20, IsNull())).WillOnce(Return(serOK));

  commandProcessingTest("EXECUTE-SCENE:10,20\n", "OK:20\n");
}

TEST_F(ExecuteSceneCommandTest, isDuringExecution) {
  EXPECT_CALL(*cmd, execute).WillOnce(Return(serIsDuringExecution));
  commandProcessingTest("EXECUTE-SCENE:10,20\n", "IS-DURING-EXECUTION:20\n");
}

TEST_F(ExecuteSceneCommandTest, notExists) {
  EXPECT_CALL(*cmd, execute).WillOnce(Return(serNotExists));
  commandProcessingTest("EXECUTE-SCENE:10,20\n", "UNKNOWN:20\n");
}

TEST_F(ExecuteSceneCommandTest, noParams) {
  EXPECT_CALL(*cmd, execute).Times(0);
  commandProcessingTest("EXECUTE-SCENE:\n", "UNKNOWN:0\n");
}

TEST_F(ExecuteSceneCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, execute).Times(0);
  commandProcessingTest("EXECUTE-SCENE:0,0\n", "UNKNOWN:0\n");
}

TEST_F(ExecuteSceneCommandTest, badParams) {
  EXPECT_CALL(*cmd, execute).Times(0);
  commandProcessingTest("EXECUTE-SCENE:a,10\n", "UNKNOWN:0\n");
}

TEST_F(ExecuteSceneCommandTest, googleRequestId) {
  EXPECT_CALL(*cmd, execute(10, 20, StrEq("RequestID")))
      .WillOnce(Return(serOK));

  commandProcessingTest("EXECUTE-SCENE:10,20,GOOGLE-REQUEST-ID=UmVxdWVzdElE\n",
                        "OK:20\n");
}

} /* namespace testing */
