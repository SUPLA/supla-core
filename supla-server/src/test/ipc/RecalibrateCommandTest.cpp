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

#include "ipc/RecalibrateCommandTest.h"

namespace testing {

void RecalibrateCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new RecalibrateCommandMock(socketAdapter);
}

void RecalibrateCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *RecalibrateCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(RecalibrateCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(RecalibrateCommandTest, RecalibrateWithSuccess) {
  EXPECT_CALL(*cmd, recalibrate(10, 20, 30)).WillOnce(Return(true));

  commandProcessingTest("RECALIBRATE:10,20,30\n", "OK:30\n");
}

TEST_F(RecalibrateCommandTest, RecalibrateWithFilure) {
  EXPECT_CALL(*cmd, recalibrate).WillOnce(Return(false));
  commandProcessingTest("RECALIBRATE:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(RecalibrateCommandTest, noParams) {
  EXPECT_CALL(*cmd, recalibrate).Times(0);
  commandProcessingTest("RECALIBRATE:\n", "UNKNOWN:0\n");
}

TEST_F(RecalibrateCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, recalibrate).Times(0);
  commandProcessingTest("RECALIBRATE:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(RecalibrateCommandTest, badParams) {
  EXPECT_CALL(*cmd, recalibrate).Times(0);
  commandProcessingTest("RECALIBRATE:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
