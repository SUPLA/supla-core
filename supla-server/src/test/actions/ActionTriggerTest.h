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

#ifndef ACTIONTRIGGERTEST_H_
#define ACTIONTRIGGERTEST_H_

#include <doubles/actions/ActionExecutorMock.h>

#include "actions/action_trigger.h"
#include "channeljsonconfig/action_trigger_config.h"
#include "doubles/device/DeviceFinderStub.h"
#include "gtest/gtest.h"  // NOLINT

namespace testing {

class ActionTriggerTest : public Test {
 protected:
  supla_action_trigger *at;
  action_trigger_config *at_config;
  ActionExecutorMock *aexec;
  DeviceFinderStub *dev_finder;

 public:
  ActionTriggerTest(void);
  virtual ~ActionTriggerTest(void);
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* ACTIONTRIGGERTEST_H_ */
