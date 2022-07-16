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

#include "ipc/IpcCommandTest.h"

namespace testing {

IpcCommandTest::IpcCommandTest() : Test() { socketAdapter = NULL; }

IpcCommandTest::~IpcCommandTest() {}

void IpcCommandTest::SetUp() {
  Test::SetUp();
  memset(buffer, 0, sizeof(buffer));
  socketAdapter = new IpcSocketAdapterMock(-1);
}

void IpcCommandTest::TearDown() {
  Test::TearDown();
  delete socketAdapter;
}

void IpcCommandTest::commandProcessingTest(const char *input,
                                           const char *expected) {
  snprintf(buffer, sizeof(buffer), input);

  EXPECT_CALL(*socketAdapter, send_data(std::string(expected))).Times(1);

  EXPECT_TRUE(getCommand()->process_command(
      buffer, sizeof(buffer), strnlen(buffer, IPC_BUFFER_MAX_SIZE)));
}

} /* namespace testing */
