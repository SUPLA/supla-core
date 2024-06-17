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

#include "TempHumConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/temp_hum_config.h"
#include "proto.h"

namespace testing {

TempHumConfigTest::TempHumConfigTest(void) {}

TempHumConfigTest::~TempHumConfigTest(void) {}

TEST_F(TempHumConfigTest, getters) {
  temp_hum_config config;
  EXPECT_FALSE(config.is_adjustment_applied_by_device());
  EXPECT_EQ(config.get_humidity_adjustment(), 0);
  EXPECT_EQ(config.get_temperature_adjustment(), 0);
  EXPECT_DOUBLE_EQ(config.get_humidity_adjustment_dbl(), 0.0);
  EXPECT_DOUBLE_EQ(config.get_temperature_adjustment_dbl(), 0.0);

  config.set_user_config(
      "{\"temperatureAdjustment\":1234,\"humidityAdjustment\":5678,"
      "\"adjustmentAppliedByDevice\":true}");

  EXPECT_TRUE(config.is_adjustment_applied_by_device());
  EXPECT_EQ(config.get_humidity_adjustment(), 5678);
  EXPECT_EQ(config.get_temperature_adjustment(), 1234);
  EXPECT_DOUBLE_EQ(config.get_humidity_adjustment_dbl(), 56.78);
  EXPECT_DOUBLE_EQ(config.get_temperature_adjustment_dbl(), 12.34);
}

TEST_F(TempHumConfigTest, setGetRawConfig) {
  temp_hum_config config;

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{}");
  free(str);

  TChannelConfig_TemperatureAndHumidity raw1 = {};
  raw1.AdjustmentAppliedByDevice = 1;
  raw1.HumidityAdjustment = 2345;
  raw1.TemperatureAdjustment = 7891;
  config.set_config(&raw1);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"temperatureAdjustment\":7891,\"humidityAdjustment\":2345,"
               "\"adjustmentAppliedByDevice\":true}");
  free(str);

  TChannelConfig_TemperatureAndHumidity raw2 = {};
  config.get_config(&raw2);

  EXPECT_EQ(memcmp(&raw1, &raw2, sizeof(TChannelConfig_TemperatureAndHumidity)),
            0);
}

TEST_F(TempHumConfigTest, merge) {
  temp_hum_config config1;
  config1.set_user_config("{\"a\":\"b\"}");

  TChannelConfig_TemperatureAndHumidity raw = {};
  raw.HumidityAdjustment = 1357;
  raw.TemperatureAdjustment = 2468;

  temp_hum_config config2;
  config2.set_config(&raw);
  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"a\":\"b\",\"temperatureAdjustment\":2468,\"humidityAdjustment\":1357,"
      "\"adjustmentAppliedByDevice\":false}");

  free(str);
}

} /* namespace testing */
