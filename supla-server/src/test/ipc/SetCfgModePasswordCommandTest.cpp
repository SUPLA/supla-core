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

#include "SetCfgModePasswordCommandTest.h"

namespace testing {

void SetCfgModePasswordCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new SetCfgModePasswordCommandMock(socketAdapter);
}

void SetCfgModePasswordCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *SetCfgModePasswordCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(SetCfgModePasswordCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SetCfgModePasswordCommandTest, setPasswordWithFailure) {
  EXPECT_CALL(*cmd, set_cfg_mode_password(10, 20, _)).WillOnce(Return(false));

  commandProcessingTest("SET-CFG-MODE-PASSWORD:10,20,YWJjZA==\n",
                        "UNKNOWN:20\n");
}

TEST_F(SetCfgModePasswordCommandTest, maximumPasswordLength) {
  EXPECT_CALL(*cmd,
              set_cfg_mode_password(10, 20,
                                    StrEq("This is the maximum length of the "
                                          "password.....................")))
      .WillOnce(Return(true));

  commandProcessingTest(
      "SET-CFG-MODE-PASSWORD:10,20,"
      "VGhpcyBpcyB0aGUgbWF4aW11bSBsZW5ndGggb2YgdGhlIHBhc3N3b3JkLi4uLi4uLi4uLi4u"
      "Li4uLi4uLi4u\n",
      "OK:20\n");
}

TEST_F(SetCfgModePasswordCommandTest, minimumPasswordLength) {
  EXPECT_CALL(*cmd, set_cfg_mode_password(10, 20, StrEq("1")))
      .WillOnce(Return(true));

  commandProcessingTest("SET-CFG-MODE-PASSWORD:10,20,MQ==\n", "OK:20\n");
}

TEST_F(SetCfgModePasswordCommandTest, passwordOneCharacterTooLong) {
  EXPECT_CALL(*cmd, set_cfg_mode_password).Times(0);

  commandProcessingTest(
      "SET-CFG-MODE-PASSWORD:10,20,"
      "UGFzc3dvcmQgb25lIGNoYXJhY3RlciB0b28gbG9uZy4uLi4uLi4uLi4uLi4uLi4uLi4uLi4u"
      "Li4uLi4uLi4uLg==\n",
      "UNKNOWN:20\n");
}

TEST_F(SetCfgModePasswordCommandTest, noParams) {
  EXPECT_CALL(*cmd, set_cfg_mode_password).Times(0);
  commandProcessingTest("SET-CFG-MODE-PASSWORD:\n", "UNKNOWN:0\n");
}

TEST_F(SetCfgModePasswordCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, set_cfg_mode_password).Times(0);
  commandProcessingTest("SET-CFG-MODE-PASSWORD:0,0,\n", "UNKNOWN:0\n");
}

TEST_F(SetCfgModePasswordCommandTest, badInteger) {
  EXPECT_CALL(*cmd, set_cfg_mode_password).Times(0);
  commandProcessingTest("SET-CFG-MODE-PASSWORD:a,10,\n", "UNKNOWN:0\n");
}

TEST_F(SetCfgModePasswordCommandTest, emptyPassword) {
  EXPECT_CALL(*cmd, set_cfg_mode_password).Times(0);
  commandProcessingTest("SET-CFG-MODE-PASSWORD:1,10,\n", "UNKNOWN:10\n");
}

} /* namespace testing */
