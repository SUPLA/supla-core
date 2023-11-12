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

#include "ActionHvacParametersTest.h"

#include "actions/action_hvac_parameters.h"
#include "device/value/channel_hvac_value.h"

namespace testing {

ActionHvacParametersTest::ActionHvacParametersTest(void) {}

ActionHvacParametersTest::~ActionHvacParametersTest(void) {}

TEST_F(ActionHvacParametersTest, applyOn_noChanges) {
  TAction_HVAC_Parameters p = {};
  supla_action_hvac_parameters hvac(&p);
  supla_channel_hvac_value v;

  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  v.get_raw_value(raw_value1);
  hvac.apply_on(&v);
  v.get_raw_value(raw_value2);

  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ActionHvacParametersTest, applyOn_mode) {
  TAction_HVAC_Parameters p = {};

  p.Mode = SUPLA_HVAC_MODE_HEAT;

  supla_action_hvac_parameters hvac(&p);
  supla_channel_hvac_value v;
  hvac.apply_on(&v);

  EXPECT_EQ(v.get_mode(), SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(v.get_temperature_heat(), 0);
  EXPECT_EQ(v.get_temperature_cool(), 0);
  EXPECT_EQ(v.get_flags(), 0);
}

TEST_F(ActionHvacParametersTest, applyOn_tempMin) {
  TAction_HVAC_Parameters p = {};

  p.SetpointTemperatureHeat = 123;
  p.Flags = SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET;

  supla_action_hvac_parameters hvac(&p);
  supla_channel_hvac_value v;
  hvac.apply_on(&v);

  EXPECT_EQ(v.get_mode(), 0);
  EXPECT_EQ(v.get_temperature_heat(), 123);
  EXPECT_EQ(v.get_temperature_cool(), 0);
  EXPECT_EQ(v.get_flags(), SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET);
}

TEST_F(ActionHvacParametersTest, applyOn_tempMax) {
  TAction_HVAC_Parameters p = {};

  p.SetpointTemperatureCool = 123;
  p.Flags = SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET;

  supla_action_hvac_parameters hvac(&p);
  supla_channel_hvac_value v;
  hvac.apply_on(&v);

  EXPECT_EQ(v.get_mode(), 0);
  EXPECT_EQ(v.get_temperature_heat(), 0);
  EXPECT_EQ(v.get_temperature_cool(), 123);
  EXPECT_EQ(v.get_flags(), SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET);
}

TEST_F(ActionHvacParametersTest, getDurationSec) {
  TAction_HVAC_Parameters p = {};
  p.DurationSec = 123;
  supla_action_hvac_parameters hvac(&p);

  EXPECT_EQ(hvac.get_duration_sec(), 123);
}

TEST_F(ActionHvacParametersTest, applyOn_allParams) {
  TAction_HVAC_Parameters p = {};

  p.DurationSec = 15;
  p.Mode = SUPLA_HVAC_MODE_COOL;
  p.SetpointTemperatureHeat = 123;
  p.Flags = SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET;
  p.SetpointTemperatureCool = 33;
  p.Flags |= SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET;

  supla_action_hvac_parameters hvac(&p);
  supla_channel_hvac_value v;

  v.set_flags(SUPLA_HVAC_VALUE_FLAG_COOLING);
  hvac.apply_on(&v);

  EXPECT_EQ(v.get_mode(), SUPLA_HVAC_MODE_COOL);
  EXPECT_EQ(v.get_temperature_heat(), 123);
  EXPECT_EQ(v.get_temperature_cool(), 33);
  EXPECT_EQ(v.get_flags(), SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET |
                               SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET |
                               SUPLA_HVAC_VALUE_FLAG_COOLING);
  EXPECT_EQ(hvac.get_duration_sec(), 15);
}

TEST_F(ActionHvacParametersTest, applyOn_minIsNull) {
  TAction_HVAC_Parameters p = {};

  p.SetpointTemperatureCool = 200;
  p.Flags |= SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET;

  supla_action_hvac_parameters hvac(&p);
  supla_channel_hvac_value v;
  v.set_temperature_heat(134);
  v.set_temperature_cool(223);

  hvac.apply_on(&v);

  EXPECT_EQ(v.get_temperature_heat(), 0);
  EXPECT_EQ(v.get_temperature_cool(), 200);
  EXPECT_EQ(v.get_flags(), SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET);
}

TEST_F(ActionHvacParametersTest, applyOn_maxIsNull) {
  TAction_HVAC_Parameters p = {};

  p.SetpointTemperatureHeat = 100;
  p.Flags |= SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET;

  supla_action_hvac_parameters hvac(&p);
  supla_channel_hvac_value v;
  v.set_temperature_heat(134);
  v.set_temperature_cool(223);

  hvac.apply_on(&v);

  EXPECT_EQ(v.get_temperature_heat(), 100);
  EXPECT_EQ(v.get_temperature_cool(), 0);
  EXPECT_EQ(v.get_flags(), SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET);
}

TEST_F(ActionHvacParametersTest, anyParamSet) {
  {
    TAction_HVAC_Parameters p = {};
    supla_action_hvac_parameters hvac(&p);
    EXPECT_FALSE(hvac.is_any_param_set());
  }

  {
    TAction_HVAC_Parameters p = {};
    p.DurationSec = 15;
    supla_action_hvac_parameters hvac(&p);
    EXPECT_FALSE(hvac.is_any_param_set());
  }

  {
    TAction_HVAC_Parameters p = {};
    p.Mode = SUPLA_HVAC_MODE_COOL;
    supla_action_hvac_parameters hvac(&p);
    EXPECT_TRUE(hvac.is_any_param_set());
  }

  {
    TAction_HVAC_Parameters p = {};
    p.Flags = SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET;
    supla_action_hvac_parameters hvac(&p);
    EXPECT_TRUE(hvac.is_any_param_set());
  }

  {
    TAction_HVAC_Parameters p = {};
    p.Flags = SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET;
    supla_action_hvac_parameters hvac(&p);
    EXPECT_TRUE(hvac.is_any_param_set());
  }
}

} /* namespace testing */
