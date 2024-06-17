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

#include "GetPnLimitCommandTest.h"

namespace testing {

void GetPnLimitCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new GetPnLimitCommandMock(socketAdapter);
}

void GetPnLimitCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *GetPnLimitCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(GetPnLimitCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(GetPnLimitCommandTest, getLimit) {
  EXPECT_CALL(*cmd, get_pn_limit(10, _)).WillOnce([](int user_id, long *left) {
    *left = -10;
    return 30;
  });

  commandProcessingTest("PN-GET-LIMIT:10\n", "PN-LIMIT:30,-10\n");
}

} /* namespace testing */
