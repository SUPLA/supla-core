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

#include "FactoryResetCommandTest.h"

namespace testing {

void FactoryResetCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new FactoryResetCommandMock(socketAdapter);
}

void FactoryResetCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *FactoryResetCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(FactoryResetCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(FactoryResetCommandTest, factoryResetWithFailure) {
  EXPECT_CALL(*cmd, factory_reset(10, 20)).WillOnce(Return(false));

  commandProcessingTest("FACTORY-RESET:10,20\n", "UNKNOWN:20\n");
}

TEST_F(FactoryResetCommandTest, factoryResetWithSuccess) {
  EXPECT_CALL(*cmd, factory_reset(10, 20)).WillOnce(Return(true));

  commandProcessingTest("FACTORY-RESET:10,20\n", "OK:20\n");
}

TEST_F(FactoryResetCommandTest, noParams) {
  EXPECT_CALL(*cmd, factory_reset).Times(0);
  commandProcessingTest("FACTORY-RESET:\n", "UNKNOWN:0\n");
}

TEST_F(FactoryResetCommandTest, paramsWithZero) {
  EXPECT_CALL(*cmd, factory_reset).Times(0);
  commandProcessingTest("FACTORY-RESET:0,10\n", "UNKNOWN:10\n");
}

TEST_F(FactoryResetCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, factory_reset).Times(0);
  commandProcessingTest("FACTORY-RESET:0,0\n", "UNKNOWN:0\n");
}

TEST_F(FactoryResetCommandTest, badParams) {
  EXPECT_CALL(*cmd, factory_reset).Times(0);
  commandProcessingTest("FACTORY-RESET:a,10\n", "UNKNOWN:0\n");
}

} /* namespace testing */
