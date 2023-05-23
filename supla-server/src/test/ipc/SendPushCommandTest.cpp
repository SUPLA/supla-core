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

#include "SendPushCommandTest.h"

namespace testing {

void SendPushCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new SendPushCommandMock(socketAdapter);
}

void SendPushCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *SendPushCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(SendPushCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(SendPushCommandTest, ok) {
  EXPECT_CALL(*cmd, send_push(10, _))
      .Times(1)
      .WillOnce([](int user_id, const char *json) {
        EXPECT_STREQ(json, "{\"body\":\"abcdfg\"}");
        return true;
      });

  commandProcessingTest("SEND-PUSH:10,eyJib2R5IjoiYWJjZGZnIn0=\n", "OK:10\n");
}

TEST_F(SendPushCommandTest, noParams) {
  EXPECT_CALL(*cmd, send_push).Times(0);
  commandProcessingTest("SEND-PUSH:\n", "FAILED:0\n");
}

TEST_F(SendPushCommandTest, noBase64EncodedString) {
  EXPECT_CALL(*cmd, send_push).Times(0);
  commandProcessingTest("SEND-PUSH:15,\n", "FAILED:15\n");
}

TEST_F(SendPushCommandTest, returnFalse) {
  EXPECT_CALL(*cmd, send_push(15, _))
      .Times(1)
      .WillOnce([](int user_id, const char *json) {
        EXPECT_STREQ(json, "{}");
        return false;
      });
  commandProcessingTest("SEND-PUSH:15,e30=\n", "FAILED:15\n");
}

} /* namespace testing */
