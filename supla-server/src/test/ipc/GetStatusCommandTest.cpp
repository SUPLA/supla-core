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

#include "ipc/GetStatusCommandTest.h"

namespace testing {

void GetStatusCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetStatusCommandMock(socketAdapter);
}

void GetStatusCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetStatusCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetStatusCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetStatusCommandTest, setBuffer) {
  EXPECT_CALL(*cmd, get_status(NotNull(), Gt(0)))
      .WillOnce([](char *buffer, unsigned int buffer_size) {
        snprintf(buffer, buffer_size, "%s", "ABCD123");
      });

  commandProcessingTest("GET-STATUS\n", "ABCD123\n");
}

} /* namespace testing */
