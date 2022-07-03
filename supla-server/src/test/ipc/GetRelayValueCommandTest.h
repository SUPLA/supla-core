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

#ifndef GETRELAYVALUECOMMANDTEST_H_
#define GETRELAYVALUECOMMANDTEST_H_

#include "doubles/ipc/GetRelayValueCommandMock.h"
#include "ipc/IpcCommandTest.h"

namespace testing {

class GetRelayValueCommandTest : public IpcCommandTest {
 protected:
  GetRelayValueCommandMock *cmd;
  virtual supla_abstract_ipc_command *getCommand(void);

 public:
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* GETRELAYVALUECOMMANDTEST_H_ */
