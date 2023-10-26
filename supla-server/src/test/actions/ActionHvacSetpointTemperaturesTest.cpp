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

#include "ActionHvacSetpointTemperaturesTest.h"

#include "actions/action_hvac_setpoint_temperatures.h"

namespace testing {

ActionHvacSetpointTemperaturesTest::ActionHvacSetpointTemperaturesTest(void) {}

ActionHvacSetpointTemperaturesTest::~ActionHvacSetpointTemperaturesTest(void) {}

TEST_F(ActionHvacSetpointTemperaturesTest, defaultConstructor) {
  supla_action_hvac_setpoint_temperatures t;
  short temperature = 0;
  EXPECT_FALSE(t.get_heating_temperature(&temperature));
  EXPECT_FALSE(t.get_cooling_temperature(&temperature));
}

TEST_F(ActionHvacSetpointTemperaturesTest, constructorWithParams) {
  short heating = 123;
  short cooling = -23;

  short temperature = 0;
  supla_action_hvac_setpoint_temperatures t1(nullptr, nullptr);
  supla_action_hvac_setpoint_temperatures t2(&heating, nullptr);
  supla_action_hvac_setpoint_temperatures t3(nullptr, &cooling);
  supla_action_hvac_setpoint_temperatures t4(&heating, &cooling);

  EXPECT_FALSE(t1.get_heating_temperature(&temperature));
  EXPECT_FALSE(t1.get_cooling_temperature(&temperature));

  EXPECT_TRUE(t2.get_heating_temperature(&temperature));
  EXPECT_EQ(temperature, 123);
  EXPECT_FALSE(t2.get_cooling_temperature(&temperature));

  temperature = 0;

  EXPECT_FALSE(t3.get_heating_temperature(&temperature));
  EXPECT_TRUE(t3.get_cooling_temperature(&temperature));
  EXPECT_EQ(temperature, -23);

  temperature = 0;

  EXPECT_TRUE(t4.get_heating_temperature(&temperature));
  EXPECT_EQ(temperature, 123);
  EXPECT_TRUE(t4.get_cooling_temperature(&temperature));
  EXPECT_EQ(temperature, -23);
}

TEST_F(ActionHvacSetpointTemperaturesTest, compare) {
  short heating = 123;
  short cooling = -23;

  supla_action_hvac_setpoint_temperatures t1(nullptr, nullptr);
  supla_action_hvac_setpoint_temperatures t2(&heating, nullptr);
  supla_action_hvac_setpoint_temperatures t3(nullptr, &cooling);
  supla_action_hvac_setpoint_temperatures t4(&heating, &cooling);
  supla_action_hvac_setpoint_temperatures t4a(&heating, &cooling);

  heating = 124;
  cooling = -24;

  supla_action_hvac_setpoint_temperatures t5(&heating, &cooling);

  EXPECT_FALSE(t1.equal(&t2));
  EXPECT_FALSE(t1.equal(&t3));
  EXPECT_FALSE(t1.equal(&t4));
  EXPECT_FALSE(t1.equal(&t5));

  EXPECT_FALSE(t2.equal(&t1));
  EXPECT_FALSE(t2.equal(&t3));
  EXPECT_FALSE(t2.equal(&t4));
  EXPECT_FALSE(t2.equal(&t5));

  EXPECT_FALSE(t3.equal(&t1));
  EXPECT_FALSE(t3.equal(&t2));
  EXPECT_FALSE(t3.equal(&t4));
  EXPECT_FALSE(t3.equal(&t5));

  EXPECT_FALSE(t4.equal(&t1));
  EXPECT_FALSE(t4.equal(&t2));
  EXPECT_FALSE(t4.equal(&t3));
  EXPECT_TRUE(t4.equal(&t4a));
  EXPECT_FALSE(t4.equal(&t5));

  EXPECT_FALSE(t5.equal(&t1));
  EXPECT_FALSE(t5.equal(&t2));
  EXPECT_FALSE(t5.equal(&t3));
  EXPECT_FALSE(t5.equal(&t4));
}

TEST_F(ActionHvacSetpointTemperaturesTest, copy) {
  short heating = 123;
  short cooling = -23;

  supla_action_hvac_setpoint_temperatures t1(nullptr, nullptr);
  supla_action_hvac_setpoint_temperatures t2(&heating, nullptr);
  supla_action_hvac_setpoint_temperatures t3(nullptr, &cooling);
  supla_action_hvac_setpoint_temperatures t4(&heating, &cooling);

  supla_abstract_action_parameters *p = t1.copy();
  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(t1.equal(p));
  delete p;

  p = t2.copy();
  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(t2.equal(p));
  delete p;

  p = t3.copy();
  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(t3.equal(p));
  delete p;

  p = t4.copy();
  ASSERT_NE(p, nullptr);
  EXPECT_TRUE(t4.equal(p));
  delete p;
}

TEST_F(ActionHvacSetpointTemperaturesTest, setGetHeatingTemperature) {
  supla_action_hvac_setpoint_temperatures t;
  short temperature = 0;
  t.set_heating_temperature(123);
  EXPECT_TRUE(t.get_heating_temperature(&temperature));
  EXPECT_EQ(temperature, 123);

  t.set_heating_temperature(nullptr);
  EXPECT_FALSE(t.get_heating_temperature(&temperature));

  temperature = 223;
  t.set_heating_temperature(&temperature);
  temperature = 0;
  EXPECT_TRUE(t.get_heating_temperature(&temperature));
  EXPECT_EQ(temperature, 223);
}

TEST_F(ActionHvacSetpointTemperaturesTest, setGetCoolingTemperature) {
  supla_action_hvac_setpoint_temperatures t;
  short temperature = 0;
  t.set_cooling_temperature(123);
  EXPECT_TRUE(t.get_cooling_temperature(&temperature));
  EXPECT_EQ(temperature, 123);

  t.set_cooling_temperature(nullptr);
  EXPECT_FALSE(t.get_cooling_temperature(&temperature));

  temperature = 223;
  t.set_cooling_temperature(&temperature);
  temperature = 0;
  EXPECT_TRUE(t.get_cooling_temperature(&temperature));
  EXPECT_EQ(temperature, 223);
}

TEST_F(ActionHvacSetpointTemperaturesTest, isAnyTemperatureSet) {
  supla_action_hvac_setpoint_temperatures t;
  EXPECT_FALSE(t.is_any_temperature_set());

  t.set_cooling_temperature(1);
  EXPECT_TRUE(t.is_any_temperature_set());
  t.set_cooling_temperature(nullptr);
  EXPECT_FALSE(t.is_any_temperature_set());

  t.set_heating_temperature(1);
  EXPECT_TRUE(t.is_any_temperature_set());
  t.set_heating_temperature(nullptr);
  EXPECT_FALSE(t.is_any_temperature_set());

  t.set_heating_temperature(1);
  t.set_cooling_temperature(2);
  EXPECT_TRUE(t.is_any_temperature_set());
}

} /* namespace testing */
