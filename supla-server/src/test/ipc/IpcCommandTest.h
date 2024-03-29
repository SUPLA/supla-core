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

#ifndef IPCCOMMANDTEST_H_
#define IPCCOMMANDTEST_H_

#include "doubles/ipc/IpcSocketAdapterMock.h"
#include "gtest/gtest.h"
#include "ipc/abstract_ipc_command.h"
#include "ipc/abstract_ipc_ctrl.h"

namespace testing {

class IpcCommandTest : public Test {
 protected:
  IpcSocketAdapterMock *socketAdapter;
  char buffer[IPC_BUFFER_MAX_SIZE];
  virtual supla_abstract_ipc_command *getCommand(void) = 0;

  void commandProcessingTest(const char *input, const char *expected);

 public:
  IpcCommandTest();
  virtual ~IpcCommandTest();
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* IPCCOMMANDTEST_H_ */
