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

#include <ipc/InterruptSceneCommandTest.h>

namespace testing {

void InterruptSceneCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new InterruptSceneCommandMock(socketAdapter);
}

void InterruptSceneCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *InterruptSceneCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(InterruptSceneCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(InterruptSceneCommandTest, interruptWithSuccess) {
  EXPECT_CALL(*cmd, interrupt(10, 20, IsNull()));

  commandProcessingTest("INTERRUPT-SCENE:10,20\n", "OK:20\n");
}

TEST_F(InterruptSceneCommandTest, noParams) {
  EXPECT_CALL(*cmd, interrupt).Times(0);
  commandProcessingTest("INTERRUPT-SCENE:\n", "UNKNOWN:0\n");
}

TEST_F(InterruptSceneCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, interrupt).Times(0);
  commandProcessingTest("INTERRUPT-SCENE:0,0\n", "UNKNOWN:0\n");
}

TEST_F(InterruptSceneCommandTest, badParams) {
  EXPECT_CALL(*cmd, interrupt).Times(0);
  commandProcessingTest("INTERRUPT-SCENE:a,10\n", "UNKNOWN:0\n");
}

TEST_F(InterruptSceneCommandTest, googleRequestId) {
  EXPECT_CALL(*cmd, interrupt(10, 20, StrEq("RequestID")));

  commandProcessingTest(
      "INTERRUPT-SCENE:10,20,GOOGLE-REQUEST-ID=UmVxdWVzdElE\n", "OK:20\n");
}

} /* namespace testing */
