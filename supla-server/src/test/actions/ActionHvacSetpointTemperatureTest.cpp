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

#include "ActionHvacSetpointTemperatureTest.h"

#include "actions/action_hvac_setpoint_temperature.h"

namespace testing {

ActionHvacSetpointTemperatureTest::ActionHvacSetpointTemperatureTest(void) {}

ActionHvacSetpointTemperatureTest::~ActionHvacSetpointTemperatureTest(void) {}

TEST_F(ActionHvacSetpointTemperatureTest, defaultConstructor) {
  supla_action_hvac_setpoint_temperature t;
  EXPECT_EQ(t.get_temperature(), 0);
}

TEST_F(ActionHvacSetpointTemperatureTest, constructorWithParam) {
  supla_action_hvac_setpoint_temperature t(123);
  EXPECT_EQ(t.get_temperature(), 123);
}

TEST_F(ActionHvacSetpointTemperatureTest, compare) {
  supla_action_hvac_setpoint_temperature t1(1);
  supla_action_hvac_setpoint_temperature t2(2);
  supla_action_hvac_setpoint_temperature t2a(2);
  supla_action_hvac_setpoint_temperature t3(3);
  EXPECT_FALSE(t1.equal(&t2));
  EXPECT_FALSE(t1.equal(&t3));
  EXPECT_FALSE(t2a.equal(&t1));
  EXPECT_TRUE(t2.equal(&t2a));
}

TEST_F(ActionHvacSetpointTemperatureTest, copy) {
  supla_action_hvac_setpoint_temperature t(234);
  supla_abstract_action_parameters* p = t.copy();
  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(t.equal(p));
  delete p;
}

} /* namespace testing */
