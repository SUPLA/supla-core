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

#include "ChannelHpThermostatValueTest.h"

#include "device/value/channel_hp_thermostat_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelHpThermostatValueTest, voidConstructor) {
  supla_channel_hp_thermostat_value v;
  EXPECT_FALSE(v.is_on());

  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  v.get_raw_value(raw_value1);
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelHpThermostatValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_hp_thermostat_value v1(raw_value);
  EXPECT_FALSE(v1.is_on());

  raw_value[0] = 1;
  supla_channel_hp_thermostat_value v2(raw_value);
  EXPECT_TRUE(v2.is_on());
}

TEST_F(ChannelHpThermostatValueTest, getTemperatures) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->PresetTemperature = 1234;
  ((TThermostat_Value *)raw_value)->MeasuredTemperature = 4456;

  supla_channel_hp_thermostat_value v1(raw_value);
  EXPECT_EQ(v1.get_preset_temperature(), 12.34);
  EXPECT_EQ(v1.get_measured_temperature(), 44.56);

  EXPECT_EQ(v1.get_preset_temperature_str(), "12.34");
  EXPECT_EQ(v1.get_measured_temperature_str(), "44.56");
}

TEST_F(ChannelHpThermostatValueTest, getHomeAssistantMode) {
  supla_channel_hp_thermostat_value v;
  EXPECT_EQ(v.get_home_assistant_mode(), "off");

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->Flags =
      HP_STATUS_POWERON | HP_STATUS_PROGRAMMODE;
  v.set_raw_value(raw_value);
  EXPECT_EQ(v.get_home_assistant_mode(), "auto");

  ((TThermostat_Value *)raw_value)->Flags = HP_STATUS_POWERON;
  v.set_raw_value(raw_value);
  EXPECT_EQ(v.get_home_assistant_mode(), "heat");
}

TEST_F(ChannelHpThermostatValueTest, getHomeAssistantAction) {
  supla_channel_hp_thermostat_value v;
  EXPECT_EQ(v.get_home_assistant_action(), "off");

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->IsOn = 1;
  v.set_raw_value(raw_value);
  EXPECT_EQ(v.get_home_assistant_action(), "off");

  ((TThermostat_Value *)raw_value)->Flags = HP_STATUS_POWERON;
  v.set_raw_value(raw_value);
  EXPECT_EQ(v.get_home_assistant_action(), "heating");

  ((TThermostat_Value *)raw_value)->IsOn = 0;
  v.set_raw_value(raw_value);
  EXPECT_EQ(v.get_home_assistant_action(), "idle");
}

}  // namespace testing
