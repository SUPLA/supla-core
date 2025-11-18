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

#include "PowerSwitchConfigTest.h"

#include "jsonconfig/channel/power_switch_config.h"
#include "test/doubles/device/CommonChannelPropertiesMock.h"

namespace testing {

PowerSwitchConfigTest::PowerSwitchConfigTest(void) {}

PowerSwitchConfigTest::~PowerSwitchConfigTest(void) {}

TEST_F(PowerSwitchConfigTest, relatedMeterChannelId) {
  power_switch_config cfg;
  TChannelConfig_PowerSwitch raw = {};
  cfg.set_config(&raw, nullptr);

  char *str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"overcurrentThreshold\":0}");
  free(str);

  str = cfg.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str,
      "{\"overcurrentMaxAllowed\":0,\"defaultRelatedMeterChannelNo\":null}");
  free(str);

  raw.DefaultRelatedMeterIsSet = 1;
  raw.DefaultRelatedMeterChannelNo = 8;
  cfg.set_config(&raw, nullptr);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"overcurrentThreshold\":0}");
  free(str);

  str = cfg.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str, "{\"overcurrentMaxAllowed\":0,\"defaultRelatedMeterChannelNo\":8}");
  free(str);

  CommonChannelPropertiesMock props1;
  CommonChannelPropertiesMock props2;
  EXPECT_CALL(props2, get_channel_number).WillOnce(Return(8));
  EXPECT_CALL(props2, get_id).WillOnce(Return(123));

  {
    InSequence s;

    EXPECT_CALL(props1, for_each).Times(1);

    EXPECT_CALL(props1, for_each)
        .WillOnce(
            [&props2](bool any_device,
                      std::function<void(
                          supla_abstract_common_channel_properties *, bool *)>
                          on_channel_properties) {
              bool will_continue = true;
              on_channel_properties(&props2, &will_continue);
              EXPECT_FALSE(will_continue);
              EXPECT_FALSE(any_device);
            });
  }

  cfg.set_config(&raw, &props1);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"overcurrentThreshold\":0}");
  free(str);

  str = cfg.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str, "{\"overcurrentMaxAllowed\":0,\"defaultRelatedMeterChannelNo\":8}");
  free(str);

  cfg.set_config(&raw, &props1);

  str = cfg.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str,
               "{\"overcurrentThreshold\":0,\"relatedMeterChannelId\":123}");
  free(str);

  str = cfg.get_properties();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(
      str, "{\"overcurrentMaxAllowed\":0,\"defaultRelatedMeterChannelNo\":8}");
  free(str);
}

TEST_F(PowerSwitchConfigTest, setAndGet) {
  power_switch_config config;
  TChannelConfig_PowerSwitch raw = {};

  raw.OvercurrentThreshold = 11;
  raw.OvercurrentMaxAllowed = 15;
  raw.DefaultRelatedMeterIsSet = 1;
  raw.DefaultRelatedMeterChannelNo = 3;
  config.set_config(&raw, nullptr);

  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.OvercurrentThreshold, 11);
  EXPECT_EQ(raw.OvercurrentMaxAllowed, 15);
  EXPECT_EQ(raw.DefaultRelatedMeterIsSet, 1);
  EXPECT_EQ(raw.DefaultRelatedMeterChannelNo, 3);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"overcurrentThreshold\":11}");
  free(str);

  str = config.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str, "{\"overcurrentMaxAllowed\":15,\"defaultRelatedMeterChannelNo\":3}");
  free(str);
}

TEST_F(PowerSwitchConfigTest, Staircase_setAndGet) {
  power_switch_config config;
  TChannelConfig_StaircaseTimer raw = {};

  raw.TimeMS = 5000;
  config.set_config(&raw, nullptr);

  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.TimeMS, 5000);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"relayTime\":50}");
  free(str);
}

TEST_F(PowerSwitchConfigTest, merge) {
  power_switch_config config1;

  config1.set_user_config(
      "{\"a\":\"b\",\"overcurrentThreshold\":0,\"relatedMeterChannelId\":123,"
      "\"relayTime\":50}");

  config1.set_properties(
      "{\"a\":\"b\",\"overcurrentMaxAllowed\":15,"
      "\"defaultRelatedMeterChannelNo\":3}");

  power_switch_config config2;

  config2.set_user_config(
      "{\"overcurrentThreshold\":15,\"relatedMeterChannelId\":200}");

  config2.set_properties(
      "{\"overcurrentMaxAllowed\":10,\"defaultRelatedMeterChannelNo\":30}");

  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"overcurrentThreshold\":15,"
               "\"relatedMeterChannelId\":123,\"relayTime\":50}");

  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"overcurrentMaxAllowed\":10,"
               "\"defaultRelatedMeterChannelNo\":30}");

  free(str);

  config1.set_user_config(
      "{\"a\":\"b\",\"overcurrentThreshold\":0,\"relatedMeterChannelId\":"
      "null}");

  config2.merge(&config1);

  str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"overcurrentThreshold\":15,"
               "\"relatedMeterChannelId\":null}");

  free(str);

  config1.set_user_config("{\"a\":\"b\",\"overcurrentThreshold\":0}");

  config2.merge(&config1);

  str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"overcurrentThreshold\":15,"
               "\"relatedMeterChannelId\":200}");

  free(str);

  config1.set_user_config(
      "{\"a\":\"b\",\"overcurrentThreshold\":15,\"relatedMeterChannelId\":123,"
      "\"relayTime\":25}");

  config2.set_user_config("{\"relayTime\":150}");

  config2.merge(&config1);

  str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"overcurrentThreshold\":15,"
               "\"relatedMeterChannelId\":123,\"relayTime\":150}");

  free(str);

  str = config1.get_properties();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"a\":\"b\",\"overcurrentMaxAllowed\":10,"
               "\"defaultRelatedMeterChannelNo\":30}");

  free(str);
}

TEST_F(PowerSwitchConfigTest, getRelatedMeterChannelId) {
  power_switch_config config;
  EXPECT_EQ(config.get_related_meter_channel_id(), 0);

  config.set_user_config("{\"relatedMeterChannelId\":null}");
  EXPECT_EQ(config.get_related_meter_channel_id(), 0);

  config.set_user_config("{\"relatedMeterChannelId\":12345}");
  EXPECT_EQ(config.get_related_meter_channel_id(), 12345);
}

TEST_F(PowerSwitchConfigTest, relayTime) {
  power_switch_config config;
  TChannelConfig_StaircaseTimer raw = {};

  config.get_config(&raw);

  EXPECT_EQ(config.get_relay_time_ms(), 0);
  EXPECT_EQ(raw.TimeMS, 0);

  config.set_user_config("{\"relayTime\":50}");

  config.get_config(&raw);

  EXPECT_EQ(config.get_relay_time_ms(), 5000);
  EXPECT_EQ(raw.TimeMS, 5000);
}

} /* namespace testing */
