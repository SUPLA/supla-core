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

#include "ChannelHvacValueTest.h"

#include <string>

#include "device/value/channel_hvac_value.h"
#include "devicechannel.h"  // NOLINT

using std::map;
using std::string;

namespace testing {

TEST_F(ChannelHvacValueTest, mode) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.set_mode(100);
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_HEAT);
}

TEST_F(ChannelHvacValueTest, turnOn) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.turn_on();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_TURN_ON);
}

TEST_F(ChannelHvacValueTest, turnOff) {
  supla_channel_hvac_value value;
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  value.turn_off();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_OFF);
}

TEST_F(ChannelHvacValueTest, switchToManual) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.switch_to_manual();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL);
}

TEST_F(ChannelHvacValueTest, switchToProgram) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_NOT_SET);
  value.switch_to_program();
  EXPECT_EQ(value.get_mode(), SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE);
}

TEST_F(ChannelHvacValueTest, clear) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  supla_channel_hvac_value value;
  value.set_setpoint_temperature_heat(1);
  value.set_setpoint_temperature_cool(2);
  value.turn_on();

  value.get_raw_value(raw_value2);
  EXPECT_NE(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);

  value.clear();

  value.get_raw_value(raw_value2);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelHvacValueTest, isOn) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_on());

  value.set_on(5);

  EXPECT_TRUE(value.is_on());
  EXPECT_EQ(value.is_on(), 5);

  value.set_on(103);

  EXPECT_EQ(value.is_on(), 102);

  value.set_on(0);
  EXPECT_FALSE(value.is_on());
  EXPECT_EQ(value.is_on(), 0);
}

TEST_F(ChannelHvacValueTest, isHeating) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_heating());

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->IsOn = 1;
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_HEATING;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.is_heating());
}

TEST_F(ChannelHvacValueTest, isCooling) {
  supla_channel_hvac_value value;
  EXPECT_FALSE(value.is_cooling());

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->IsOn = 1;
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_COOLING;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.is_cooling());
}

TEST_F(ChannelHvacValueTest, getTemperature) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_setpoint_temperature(), 0);
  EXPECT_DOUBLE_EQ(value.get_setpoint_temperature_dbl(), 0.0);

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->SetpointTemperatureCool = 1234;
  ((THVACValue*)raw_value)->SetpointTemperatureHeat = 5678;
  value.set_raw_value(raw_value);

  EXPECT_DOUBLE_EQ(value.get_setpoint_temperature_cool_dbl(), 12.34);
  EXPECT_EQ(value.get_setpoint_temperature_cool_str(), "12.34");

  EXPECT_DOUBLE_EQ(value.get_setpoint_temperature_heat_dbl(), 56.78);
  EXPECT_EQ(value.get_setpoint_temperature_heat_str(), "56.78");

  EXPECT_EQ(value.get_setpoint_temperature(), 5678);
  EXPECT_DOUBLE_EQ(value.get_setpoint_temperature_dbl(), 56.78);
  EXPECT_EQ(value.get_setpoint_temperature_str(), "56.78");

  value.set_mode(SUPLA_HVAC_MODE_COOL);

  EXPECT_EQ(value.get_setpoint_temperature(), 1234);
  EXPECT_DOUBLE_EQ(value.get_setpoint_temperature_dbl(), 12.34);
  EXPECT_EQ(value.get_setpoint_temperature_str(), "12.34");
}

TEST_F(ChannelHvacValueTest, getHomeAssistantMode) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_home_assistant_mode(), "off");
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(value.get_home_assistant_mode(), "heat");
  value.set_mode(SUPLA_HVAC_MODE_COOL);
  EXPECT_EQ(value.get_home_assistant_mode(), "cool");
  value.set_mode(SUPLA_HVAC_MODE_HEAT_COOL);
  EXPECT_EQ(value.get_home_assistant_mode(), "heat_cool");
  value.set_flags(SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE);
  EXPECT_EQ(value.get_home_assistant_mode(), "auto");
}

TEST_F(ChannelHvacValueTest, getHomeAssistantAction) {
  supla_channel_hvac_value value;

  value.set_mode(SUPLA_HVAC_MODE_NOT_SET);
  EXPECT_EQ(value.get_home_assistant_action(), "off");

  value.set_mode(SUPLA_HVAC_MODE_OFF);
  EXPECT_EQ(value.get_home_assistant_action(), "off");

  value.set_flags(
      SUPLA_HVAC_VALUE_FLAG_HEATING);  // It can heat regardless of the mode
  EXPECT_EQ(value.get_home_assistant_action(), "heating");

  value.set_flags(
      SUPLA_HVAC_VALUE_FLAG_COOLING);  // It can cool regardless of the mode
  EXPECT_EQ(value.get_home_assistant_action(), "cooling");

  value.set_flags(0);
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(value.get_home_assistant_action(), "idle");
}

TEST_F(ChannelHvacValueTest, getGoogleHomeMode) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_google_home_mode(), "off");
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(value.get_google_home_mode(), "heat");
  value.set_mode(SUPLA_HVAC_MODE_COOL);
  EXPECT_EQ(value.get_google_home_mode(), "cool");
  value.set_mode(SUPLA_HVAC_MODE_HEAT_COOL);
  EXPECT_EQ(value.get_google_home_mode(), "heatcool");
  value.set_flags(SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE);
  EXPECT_EQ(value.get_google_home_mode(), "auto");
}

TEST_F(ChannelHvacValueTest, getAlexaMode) {
  supla_channel_hvac_value value;
  EXPECT_EQ(value.get_alexa_mode(), "OFF");
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  EXPECT_EQ(value.get_alexa_mode(), "HEAT");
  value.set_mode(SUPLA_HVAC_MODE_COOL);
  EXPECT_EQ(value.get_alexa_mode(), "COOL");
  value.set_mode(SUPLA_HVAC_MODE_HEAT_COOL);
  EXPECT_EQ(value.get_alexa_mode(), "AUTO");
}

TEST_F(ChannelHvacValueTest, replacementMap) {
  supla_channel_hvac_value value;

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->SetpointTemperatureCool = 1234;
  ((THVACValue*)raw_value)->SetpointTemperatureHeat = 5678;
  value.set_raw_value(raw_value);
  value.set_mode(SUPLA_HVAC_MODE_HEAT);
  value.set_flags(SUPLA_HVAC_VALUE_FLAG_THERMOMETER_ERROR |
                  SUPLA_HVAC_VALUE_FLAG_CLOCK_ERROR);

  map<string, string> m = value.get_replacement_map();
  EXPECT_EQ(m.size(), 5);
  EXPECT_EQ(m["mode"], "HEAT");
  EXPECT_EQ(m["setpoint_temperature_cool"], "12.34");
  EXPECT_EQ(m["setpoint_temperature_heat"], "56.78");
  EXPECT_EQ(m["heating_or_cooling"], "IDLE");
  EXPECT_EQ(m["errors"], "THERMOMETER_ERROR, CLOCK_ERROR");
}

TEST_F(ChannelHvacValueTest, getVbtValue) {
  supla_channel_hvac_value value;

  double vbt_value = 0;
  EXPECT_FALSE(value.get_vbt_value(var_name_none, &vbt_value));

  EXPECT_TRUE(value.get_vbt_value(var_name_heating, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_cooling, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_heating_or_cooling, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_on, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_battery_cover_open, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_thermometer_error, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_clock_error, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue*)raw_value)->IsOn = 1;
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_HEATING;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.get_vbt_value(var_name_heating, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  EXPECT_TRUE(value.get_vbt_value(var_name_heating_or_cooling, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  memset(raw_value, 0, sizeof(raw_value));
  ((THVACValue*)raw_value)->IsOn = 1;
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_COOLING;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.get_vbt_value(var_name_cooling, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  EXPECT_TRUE(value.get_vbt_value(var_name_heating_or_cooling, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  memset(raw_value, 0, sizeof(raw_value));
  ((THVACValue*)raw_value)->IsOn = 1;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_on, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  memset(raw_value, 0, sizeof(raw_value));
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_BATTERY_COVER_OPEN;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_battery_cover_open, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  memset(raw_value, 0, sizeof(raw_value));
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_THERMOMETER_ERROR;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.get_vbt_value(var_name_thermometer_error, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  memset(raw_value, 0, sizeof(raw_value));
  ((THVACValue*)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_CLOCK_ERROR;
  value.set_raw_value(raw_value);

  EXPECT_TRUE(value.get_vbt_value(var_name_clock_error, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 1);
}

}  // namespace testing
