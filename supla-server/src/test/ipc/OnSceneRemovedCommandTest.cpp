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

#include <ipc/OnSceneRemovedCommandTest.h>

namespace testing {

void OnSceneRemovedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnSceneRemovedCommandMock(socketAdapter);
}

void OnSceneRemovedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnSceneRemovedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OnSceneRemovedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnSceneRemovedCommandTest, onSceneAdded) {
  EXPECT_CALL(*cmd, on_scene_removed(10, 20)).Times(1);

  commandProcessingTest("USER-ON-SCENE-REMOVED:10,20\n", "OK:10\n");
}

TEST_F(OnSceneRemovedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_scene_removed).Times(0);
  commandProcessingTest("USER-ON-SCENE-REMOVED:\n", "UNKNOWN:0\n");
}

TEST_F(OnSceneRemovedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, on_scene_removed).Times(0);
  commandProcessingTest("USER-ON-SCENE-REMOVED:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OnSceneRemovedCommandTest, badParams) {
  EXPECT_CALL(*cmd, on_scene_removed).Times(0);
  commandProcessingTest("USER-ON-SCENE-REMOVED:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
