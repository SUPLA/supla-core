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

#include <ipc/OnSceneAddedCommandTest.h>

namespace testing {

void OnSceneAddedCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new OnSceneAddedCommandMock(socketAdapter);
}

void OnSceneAddedCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *OnSceneAddedCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(OnSceneAddedCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(OnSceneAddedCommandTest, onSceneAdded) {
  EXPECT_CALL(*cmd, on_scene_added(10, 20)).Times(1);

  commandProcessingTest("USER-ON-SCENE-ADDED:10,20\n", "OK:10\n");
}

TEST_F(OnSceneAddedCommandTest, noParams) {
  EXPECT_CALL(*cmd, on_scene_added).Times(0);
  commandProcessingTest("USER-ON-SCENE-ADDED:\n", "UNKNOWN:0\n");
}

TEST_F(OnSceneAddedCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, on_scene_added).Times(0);
  commandProcessingTest("USER-ON-SCENE-ADDED:0,0\n", "UNKNOWN:0\n");
}

TEST_F(OnSceneAddedCommandTest, badParams) {
  EXPECT_CALL(*cmd, on_scene_added).Times(0);
  commandProcessingTest("USER-ON-SCENE-ADDED:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
