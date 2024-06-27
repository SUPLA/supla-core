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

#include "ipc/PairSubdeviceCommandTest.h"

namespace testing {

void PairSubdeviceCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new PairSubdeviceCommandMock(socketAdapter);
}

void PairSubdeviceCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *PairSubdeviceCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(PairSubdeviceCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(PairSubdeviceCommandTest, RecalibrateWithSuccess) {
  EXPECT_CALL(*cmd, pair_subdevice(10, 20)).WillOnce(Return(true));

  commandProcessingTest("PAIR-SUBDEVICE:10,20\n", "OK:20\n");
}

TEST_F(PairSubdeviceCommandTest, RecalibrateWithFilure) {
  EXPECT_CALL(*cmd, pair_subdevice).WillOnce(Return(false));
  commandProcessingTest("PAIR-SUBDEVICE:10,20\n", "FAIL:20\n");
}

TEST_F(PairSubdeviceCommandTest, noParams) {
  EXPECT_CALL(*cmd, pair_subdevice).Times(0);
  commandProcessingTest("PAIR-SUBDEVICE:\n", "FAIL:0\n");
}

TEST_F(PairSubdeviceCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, pair_subdevice).Times(0);
  commandProcessingTest("PAIR-SUBDEVICE:0,0\n", "FAIL:0\n");
}

TEST_F(PairSubdeviceCommandTest, badParams) {
  EXPECT_CALL(*cmd, pair_subdevice).Times(0);
  commandProcessingTest("PAIR-SUBDEVICE:a,10\n", "FAIL:0\n");
}

} /* namespace testing */
