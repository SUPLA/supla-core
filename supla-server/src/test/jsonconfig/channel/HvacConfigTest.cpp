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

#include "HvacConfigTest.h"

#include "TestHelper.h"
#include "json/cJSON.h"
#include "jsonconfig/channel/hvac_config.h"
#include "log.h"
#include "proto.h"

namespace testing {

HvacConfigTest::HvacConfigTest(void) {}

HvacConfigTest::~HvacConfigTest(void) {}

TEST_F(HvacConfigTest, setAndGetConfig) {
  TChannelConfig_HVAC ds_hvac1 = {};
  ds_hvac1.MainThermometerChannelNo = 1;
  ds_hvac1.AuxThermometerChannelNo = 2;
  ds_hvac1.BinarySensorChannelNo = 3;
  ds_hvac1.AuxThermometerType = SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER;
  ds_hvac1.AntiFreezeAndOverheatProtectionEnabled = true;
  ds_hvac1.AvailableAlgorithms = 0xFFFF;
  ds_hvac1.UsedAlgorithm = SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_MIDDLE;
  ds_hvac1.MinOnTimeS = 10;
  ds_hvac1.MinOffTimeS = 600;
  ds_hvac1.OutputValueOnError = 55;
  ds_hvac1.TemperatureSetpointChangeSwitchesToManualMode = 1;
  ds_hvac1.Subfunction = SUPLA_HVAC_SUBFUNCTION_COOL;

  int size = sizeof(ds_hvac1.Temperatures.Temperature) / sizeof(_supla_int16_t);

  for (int a = 0; a < size; a++) {
    ds_hvac1.Temperatures.Index |= 1 << a;
    ds_hvac1.Temperatures.Temperature[a] = a + 1;
  }

  hvac_config config1;
  config1.set_config(&ds_hvac1, 0);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"mainThermometerChannelNo\":1,\"auxThermometerChannelNo\":2,"
      "\"auxThermometerType\":\"GENERIC_HEATER\",\"binarySensorChannelNo\":3,"
      "\"antiFreezeAndOverheatProtectionEnabled\":true,\"usedAlgorithm\":\"ON_"
      "OFF_SETPOINT_MIDDLE\",\"minOffTimeS\":600,\"minOnTimeS\":10,"
      "\"outputValueOnError\":55,\"subfunction\":\"COOL\","
      "\"temperatureSetpointChangeSwitchesToManualMode\":true,\"temperatures\":"
      "{\"freezeProtection\":1,\"eco\":2,\"comfort\":3,\"boost\":4,"
      "\"heatProtection\":5,\"histeresis\":6,\"belowAlarm\":7,\"aboveAlarm\":8,"
      "\"auxMinSetpoint\":9,\"auxMaxSetpoint\":10}}");

  hvac_config config2;
  config2.set_user_config(str);
  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"availableAlgorithms\":[\"ON_OFF_SETPOINT_MIDDLE\",\"ON_OFF_SETPOINT_"
      "AT_MOST\"],\"temperatures\":{\"roomMin\":11,\"roomMax\":12,\"auxMin\":"
      "13,\"auxMax\":14,\"histeresisMin\":15,\"histeresisMax\":16,"
      "\"autoOffsetMin\":17,\"autoOffsetMax\":18}}");

  config2.set_properties(str);
  free(str);

  TChannelConfig_HVAC ds_hvac2 = {};
  config2.get_config(&ds_hvac2, 0);

  // Set unused to zero
  ds_hvac1.Temperatures.Index = ds_hvac2.Temperatures.Index;
  for (int a = 0; a < size; a++) {
    if (!(ds_hvac1.Temperatures.Index & (1 << a))) {
      ds_hvac1.Temperatures.Temperature[a] = 0;
    }
  }

  ds_hvac1.AvailableAlgorithms = SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_MIDDLE |
                                 SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_AT_MOST;

  EXPECT_EQ(memcmp(&ds_hvac1, &ds_hvac2, sizeof(TChannelConfig_HVAC)), 0);

  EXPECT_EQ(ds_hvac1.Temperatures.Index, ds_hvac2.Temperatures.Index);
}

TEST_F(HvacConfigTest, getConfigResult) {
  hvac_config config;
  TChannelConfig_HVAC ds_hvac = {};
  EXPECT_FALSE(config.get_config(&ds_hvac, 0));

  config.set_user_config("{}");

  EXPECT_FALSE(config.get_config(&ds_hvac, 0));

  config.set_user_config("{\"minOffTimeS\":600}");

  EXPECT_TRUE(config.get_config(&ds_hvac, 0));
}

TEST_F(HvacConfigTest, getUnsetChannelNumber) {
  hvac_config config;
  TChannelConfig_HVAC ds_hvac = {};

  EXPECT_FALSE(config.get_config(&ds_hvac, 10));
  EXPECT_EQ(ds_hvac.BinarySensorChannelNo, 10);

  config.set_user_config("{\"binarySensorChannelNo\":null}");

  EXPECT_TRUE(config.get_config(&ds_hvac, 15));
  EXPECT_EQ(ds_hvac.BinarySensorChannelNo, 15);
}

TEST_F(HvacConfigTest, selectedTemperatures) {
  TChannelConfig_HVAC ds_hvac = {};

  ds_hvac.Temperatures.Index = (1 << 0) | (1 << 1) | (1 << 8) | (1 << 15);
  int size = sizeof(ds_hvac.Temperatures.Temperature) / sizeof(_supla_int16_t);

  for (int a = 0; a < size; a++) {
    ds_hvac.Temperatures.Temperature[a] = a + 1;
  }

  ds_hvac.Temperatures.Temperature[0] = 12345;
  ds_hvac.Temperatures.Temperature[1] = 0;
  ds_hvac.Temperatures.Temperature[8] = -723;
  ds_hvac.Temperatures.Temperature[15] = -28910;

  hvac_config config;
  config.set_config(&ds_hvac, 0);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"mainThermometerChannelNo\":null,\"auxThermometerChannelNo\":null,"
      "\"auxThermometerType\":\"NOT_SET\",\"binarySensorChannelNo\":null,"
      "\"antiFreezeAndOverheatProtectionEnabled\":false,\"usedAlgorithm\":\"\","
      "\"minOffTimeS\":0,\"minOnTimeS\":0,\"outputValueOnError\":0,"
      "\"subfunction\":\"NOT_SET\","
      "\"temperatureSetpointChangeSwitchesToManualMode\":false,"
      "\"temperatures\":{\"freezeProtection\":12345,\"eco\":0,"
      "\"auxMinSetpoint\":-723}}");

  free(str);

  str = config.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"availableAlgorithms\":[],\"temperatures\":{"
               "\"histeresisMax\":-28910}}");

  free(str);
}

TEST_F(HvacConfigTest, merge) {
  hvac_config config1;
  config1.set_user_config(
      "{\"a\":\"b\", \"x\": true, \"c\": true, "
      "\"mainThermometerChannelNo\":1,\"auxThermometerChannelNo\":2,"
      "\"auxThermometerType\":\"GENERIC_HEATER\","
      "\"antiFreezeAndOverheatProtectionEnabled\":true,\"usedAlgorithm\":\"ON_"
      "OFF_SETPOINT_MIDDLE\",\"minOffTimeS\":600,\"minOnTimeS\":10,"
      "\"outputValueOnError\":55,\"temperatures\":{\"freezeProtection\":1,"
      "\"eco\":2,\"comfort\":3,\"boost\":4,\"heatProtection\":5,\"histeresis\":"
      "6,\"belowAlarm\":7,\"aboveAlarm\":8,\"auxMinSetpoint\":9,"
      "\"auxMaxSetpoint\":10}}");

  config1.set_properties(
      "{\"1\": "
      "10,\"availableAlgorithms\":[\"ON_OFF_SETPOINT_MIDDLE\"],"
      "\"temperatures\":{\"roomMin\":11,\"roomMax\":12,\"auxMin\":13,"
      "\"auxMax\":14,\"histeresisMin\":15,\"histeresisMax\":16,"
      "\"autoOffsetMin\":17,\"autoOffsetMax\":18}}");

  TChannelConfig_HVAC ds_hvac = {};

  ds_hvac.Temperatures.Index = 1ULL << 1;
  ds_hvac.Temperatures.Index |= 1ULL << 2;
  ds_hvac.Temperatures.Index |= 1ULL << 16;
  ds_hvac.Temperatures.Index |= 1ULL << 17;
  ds_hvac.Temperatures.Temperature[1] = 10;
  ds_hvac.Temperatures.Temperature[17] = 19;

  hvac_config config2;
  config2.set_config(&ds_hvac, 0);
  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"a\":\"b\",\"x\":true,\"c\":true,\"mainThermometerChannelNo\":null,"
      "\"auxThermometerChannelNo\":null,\"auxThermometerType\":\"NOT_SET\","
      "\"antiFreezeAndOverheatProtectionEnabled\":false,\"usedAlgorithm\":\"\","
      "\"minOffTimeS\":0,\"minOnTimeS\":0,\"outputValueOnError\":0,"
      "\"temperatures\":{\"eco\":10,\"comfort\":0},\"binarySensorChannelNo\":"
      "null,\"subfunction\":\"NOT_SET\","
      "\"temperatureSetpointChangeSwitchesToManualMode\":false}");

  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"1\":10,\"availableAlgorithms\":[],\"temperatures\":{"
               "\"autoOffsetMin\":0,\"autoOffsetMax\":19}}");

  free(str);
}

} /* namespace testing */
