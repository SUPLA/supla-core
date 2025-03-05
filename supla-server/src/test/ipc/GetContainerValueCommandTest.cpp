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

#include "GetContainerValueCommandTest.h"

namespace testing {

void GetContainerValueCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetContainerValueCommandMock(socketAdapter);
}

void GetContainerValueCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetContainerValueCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetContainerValueCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetContainerValueCommandTest, getContainerValueWithSuccess) {
  TContainerChannel_Value raw_value = {};
  raw_value.level = 10;
  raw_value.flags = 123;

  supla_channel_container_value *value =
      new supla_channel_container_value(&raw_value);

  EXPECT_CALL(*cmd, get_value(10, 20, 30)).WillOnce(Return(value));

  commandProcessingTest("GET-CONTAINER-VALUE:10,20,30\n", "VALUE:10,123\n");
}

TEST_F(GetContainerValueCommandTest, getContainerValueWithFilure) {
  supla_channel_container_value *value = nullptr;
  EXPECT_CALL(*cmd, get_value).WillOnce(Return(value));
  commandProcessingTest("GET-CONTAINER-VALUE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(GetContainerValueCommandTest, noParams) {
  EXPECT_CALL(*cmd, get_value).Times(0);
  commandProcessingTest("GET-CONTAINER-VALUE:\n", "UNKNOWN:0\n");
}

TEST_F(GetContainerValueCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, get_value).Times(0);
  commandProcessingTest("GET-CONTAINER-VALUE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(GetContainerValueCommandTest, badParams) {
  EXPECT_CALL(*cmd, get_value).Times(0);
  commandProcessingTest("GET-CONTAINER-VALUE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
