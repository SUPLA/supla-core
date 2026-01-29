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

#ifndef SETRGBWCOMMANDTEST_H_
#define SETRGBWCOMMANDTEST_H_

#include "doubles/actions/ActionExecutorGmock.h"
#include "ipc/IpcCommandTest.h"
#include "ipc/set_rgbw_command.h"

namespace testing {

class SetRGBWCommandTest : public IpcCommandTest {
 protected:
  ActionExecutorGmock actionExecutor;
  supla_abstract_ipc_command *cmd;
  supla_set_rgbw_command *setCmd;
  supla_set_rgbw_command *setRandCmd;
  supla_set_rgbw_command *cgSetCmd;
  supla_set_rgbw_command *cgSetRandCmd;

  supla_abstract_ipc_command *getCommand(void);

 public:
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* SETRGBWCOMMANDTEST_H_ */
