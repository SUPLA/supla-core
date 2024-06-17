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

#include "BinarySensorConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/binary_sensor_config.h"
#include "proto.h"

namespace testing {

BinarySensorConfigTest::BinarySensorConfigTest(void) {}

BinarySensorConfigTest::~BinarySensorConfigTest(void) {}

TEST_F(BinarySensorConfigTest, isInvertedLogic) {
  binary_sensor_config config;

  EXPECT_FALSE(config.is_logic_inverted());

  config.set_user_config("{\"invertedLogic\":false}");

  EXPECT_FALSE(config.is_logic_inverted());

  config.set_user_config("{\"invertedLogic\":true}");

  EXPECT_TRUE(config.is_logic_inverted());
}

TEST_F(BinarySensorConfigTest, filteringTimeMs) {
  binary_sensor_config config;

  EXPECT_FALSE(config.is_logic_inverted());

  config.set_user_config("{\"filteringTimeMs\":0}");

  EXPECT_EQ(config.get_filtering_time_ms(), 0);

  config.set_user_config("{\"filteringTimeMs\":123}");

  EXPECT_EQ(config.get_filtering_time_ms(), 123);
}

TEST_F(BinarySensorConfigTest, setGetRawConfig) {
  binary_sensor_config config;

  EXPECT_FALSE(config.is_logic_inverted());

  TChannelConfig_BinarySensor raw = {};
  raw.InvertedLogic = 1;
  raw.FilteringTimeMs = 22;
  config.set_config(&raw);

  EXPECT_TRUE(config.is_logic_inverted());
  EXPECT_EQ(config.get_filtering_time_ms(), 22);

  raw = {};
  config.get_config(&raw);
  EXPECT_EQ(raw.InvertedLogic, 1);
  EXPECT_EQ(raw.FilteringTimeMs, 22);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"invertedLogic\":true,\"filteringTimeMs\":22}");
  free(str);
}

TEST_F(BinarySensorConfigTest, merge) {
  binary_sensor_config config1;
  config1.set_user_config(
      "{\"a\":\"b\", \"invertedLogic\": false,\"filteringTimeMs\":0}");

  TChannelConfig_BinarySensor raw_bin = {};
  raw_bin.InvertedLogic = 1;
  raw_bin.FilteringTimeMs = 5;

  binary_sensor_config config2;
  config2.set_config(&raw_bin);
  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"invertedLogic\":true,\"filteringTimeMs\":5}");

  free(str);
}

} /* namespace testing */
