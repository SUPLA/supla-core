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

#include "ValveConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/valve_config.h"

namespace testing {

ValveConfigTest::ValveConfigTest(void) {}

ValveConfigTest::~ValveConfigTest(void) {}

TEST_F(ValveConfigTest, setAndGetConfig) {
  TChannelConfig_Valve raw1 = {};

  for (size_t a = 0; a < sizeof(raw1.SensorInfo) / sizeof(TValve_SensorInfo);
       a++) {
    raw1.SensorInfo[a].ChannelNo = a;
    raw1.SensorInfo[a].IsSet = 1;
  }

  valve_config config1;
  config1.set_config(&raw1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"sensorChannelNumbers\":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,"
               "15,16,17,18,19]}");

  valve_config config2;
  config2.set_user_config(str);
  free(str);

  TChannelConfig_Valve raw2 = {};
  EXPECT_TRUE(config2.get_config(&raw2));

  EXPECT_EQ(0, memcmp(&raw1, &raw2, sizeof(raw1)));
}

TEST_F(ValveConfigTest, duplicateAndOversize) {
  const char cfg[] =
      "{\"sensorChannelNumbers\":[0,1,2,3,4,5,6,7,8,8,8,8,9,10,11,12,13,14,15,"
      "16,17,18,19,20,30]}";

  valve_config config;
  config.set_user_config(cfg);

  TChannelConfig_Valve raw = {};
  EXPECT_TRUE(config.get_config(&raw));

  for (size_t a = 0; a < sizeof(raw.SensorInfo) / sizeof(TValve_SensorInfo);
       a++) {
    EXPECT_EQ(raw.SensorInfo[a].ChannelNo, a);
    EXPECT_EQ(raw.SensorInfo[a].IsSet, 1);
  }
}

TEST_F(ValveConfigTest, duplicateRaw) {
  TChannelConfig_Valve raw = {};

  raw.SensorInfo[1].ChannelNo = 10;
  raw.SensorInfo[1].IsSet = 1;

  raw.SensorInfo[2].ChannelNo = 5;
  raw.SensorInfo[2].IsSet = 1;

  raw.SensorInfo[3].ChannelNo = 15;
  raw.SensorInfo[3].IsSet = 1;

  raw.SensorInfo[5].ChannelNo = 5;
  raw.SensorInfo[5].IsSet = 1;

  valve_config config;
  config.set_config(&raw);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str, "{\"sensorChannelNumbers\":[10,5,15]}");

  free(str);
}

TEST_F(ValveConfigTest, merge) {
  valve_config cfg1, cfg2;

  cfg1.set_user_config(
      "{\"yxyz\":123,\"abcd\":567,\"sensorChannelNumbers\":[10,5,15]}");

  cfg2.set_user_config("{\"sensorChannelNumbers\":[3]}");

  cfg2.merge(&cfg1);

  char *str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"yxyz\":123,\"abcd\":567,\"sensorChannelNumbers\":[3]}");
  free(str);
}

} /* namespace testing */
