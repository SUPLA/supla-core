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

#include "ContainerConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/container_config.h"

namespace testing {

ContainerConfigTest::ContainerConfigTest(void) {}

ContainerConfigTest::~ContainerConfigTest(void) {}

TEST_F(ContainerConfigTest, setAndGetConfig) {
  TChannelConfig_Container raw1 = {};
  raw1.WarningAboveLevel = 20;
  raw1.AlarmAboveLevel = 30;
  raw1.WarningBelowLevel = 50;
  raw1.AlarmBelowLevel = 40;

  for (size_t a = 0;
       a < sizeof(raw1.SensorInfo) / sizeof(TContainer_SensorInfo); a++) {
    raw1.SensorInfo[a].FillLevel = 11 + a;
    raw1.SensorInfo[a].ChannelNo = a;
    raw1.SensorInfo[a].IsSet = 1;
  }

  container_config config1;
  config1.set_config(&raw1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"warningAboveLevel\":19,\"alarmAboveLevel\":29,\"warningBelowLevel\":"
      "49,\"alarmBelowLevel\":39,\"muteAlarmSoundWithoutAdditionalAuth\":false,"
      "\"sensors\":{\"0\":{\"fillLevel\":11},\"1\":{\"fillLevel\":12},\"2\":{"
      "\"fillLevel\":13},\"3\":{\"fillLevel\":14},\"4\":{\"fillLevel\":15},"
      "\"5\":{\"fillLevel\":16},\"6\":{\"fillLevel\":17},\"7\":{\"fillLevel\":"
      "18},\"8\":{\"fillLevel\":19},\"9\":{\"fillLevel\":20}}}");

  container_config config2;
  config2.set_user_config(str);
  free(str);

  TChannelConfig_Container raw2 = {};
  EXPECT_TRUE(config2.get_config(&raw2));

  EXPECT_EQ(0, memcmp(&raw1, &raw2, sizeof(raw1)));
}

TEST_F(ContainerConfigTest, null) {
  TChannelConfig_Container raw1 = {};

  container_config config1;
  config1.set_config(&raw1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"warningAboveLevel\":null,\"alarmAboveLevel\":null,"
               "\"warningBelowLevel\":null,\"alarmBelowLevel\":null,"
               "\"muteAlarmSoundWithoutAdditionalAuth\":false,\"sensors\":{}}");

  container_config config2;
  config2.set_user_config(str);
  free(str);

  TChannelConfig_Container raw2 = {};
  EXPECT_TRUE(config2.get_config(&raw2));

  EXPECT_EQ(0, memcmp(&raw1, &raw2, sizeof(raw1)));
}

TEST_F(ContainerConfigTest, oversize) {
  const char cfg[] =
      "{\"sensors\":{\"0\":{\"fillLevel\":11},\"1\":{\"fillLevel\":12},\"2\":{"
      "\"fillLevel\":13},\"3\":{\"fillLevel\":14},\"4\":{\"fillLevel\":15},"
      "\"5\":{\"fillLevel\":16},\"6\":{\"fillLevel\":17},\"7\":{\"fillLevel\":"
      "18},\"8\":{\"fillLevel\":19},\"9\":{\"fillLevel\":20},\"10\":{"
      "\"fillLevel\":20}}}";

  container_config config;
  config.set_user_config(cfg);

  TChannelConfig_Container raw = {};
  EXPECT_TRUE(config.get_config(&raw));

  for (size_t a = 0; a < sizeof(raw.SensorInfo) / sizeof(TContainer_SensorInfo);
       a++) {
    EXPECT_EQ(raw.SensorInfo[a].FillLevel, 11 + a);
    EXPECT_EQ(raw.SensorInfo[a].ChannelNo, a);
    EXPECT_EQ(raw.SensorInfo[a].IsSet, 1);
  }
}

TEST_F(ContainerConfigTest, duplicateJson) {
  const char cfg[] =
      "{\"sensors\":{\"1\":{\"fillLevel\":11},\"1\":{\"fillLevel\":12}}}";

  container_config config;
  config.set_user_config(cfg);

  TChannelConfig_Container raw = {};
  EXPECT_TRUE(config.get_config(&raw));

  for (size_t a = 0; a < sizeof(raw.SensorInfo) / sizeof(TContainer_SensorInfo);
       a++) {
    if (a == 0) {
      EXPECT_EQ(raw.SensorInfo[a].FillLevel, 12);
      EXPECT_EQ(raw.SensorInfo[a].ChannelNo, 1);
      EXPECT_EQ(raw.SensorInfo[a].IsSet, 1);
    } else {
      EXPECT_EQ(raw.SensorInfo[a].FillLevel, 0);
      EXPECT_EQ(raw.SensorInfo[a].ChannelNo, 0);
      EXPECT_EQ(raw.SensorInfo[a].IsSet, 0);
    }
  }
}

TEST_F(ContainerConfigTest, duplicateRaw) {
  TChannelConfig_Container raw = {};

  raw.SensorInfo[1].FillLevel = 11;
  raw.SensorInfo[1].ChannelNo = 10;
  raw.SensorInfo[1].IsSet = 1;

  raw.SensorInfo[2].FillLevel = 11;
  raw.SensorInfo[2].ChannelNo = 5;
  raw.SensorInfo[2].IsSet = 1;

  raw.SensorInfo[3].FillLevel = 8;
  raw.SensorInfo[3].ChannelNo = 15;
  raw.SensorInfo[3].IsSet = 1;

  raw.SensorInfo[5].FillLevel = 21;
  raw.SensorInfo[5].ChannelNo = 5;
  raw.SensorInfo[5].IsSet = 1;

  container_config config;
  config.set_config(&raw);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"warningAboveLevel\":null,\"alarmAboveLevel\":null,"
      "\"warningBelowLevel\":null,\"alarmBelowLevel\":null,"
      "\"muteAlarmSoundWithoutAdditionalAuth\":false,\"sensors\":{\"10\":{"
      "\"fillLevel\":11},\"15\":{\"fillLevel\":8},\"5\":{\"fillLevel\":21}}}");

  free(str);
}

TEST_F(ContainerConfigTest, mute) {
  TChannelConfig_Container raw1 = {};
  raw1.MuteAlarmSoundWithoutAdditionalAuth = 1;

  container_config config1;
  config1.set_config(&raw1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"warningAboveLevel\":null,\"alarmAboveLevel\":null,"
               "\"warningBelowLevel\":null,\"alarmBelowLevel\":null,"
               "\"muteAlarmSoundWithoutAdditionalAuth\":true,\"sensors\":{}}");

  container_config config2;
  config2.set_user_config(str);
  free(str);

  TChannelConfig_Container raw2 = {};
  EXPECT_TRUE(config2.get_config(&raw2));

  EXPECT_EQ(0, memcmp(&raw1, &raw2, sizeof(raw1)));
}

TEST_F(ContainerConfigTest, merge) {
  container_config cfg1, cfg2;

  cfg1.set_user_config("{\"yxyz\":123,\"abcd\":567,\"warningAboveLevel\":85}");

  cfg2.set_user_config("{\"warningAboveLevel\":10}");

  cfg2.merge(&cfg1);

  char *str = cfg1.get_user_config();
  ASSERT_TRUE(str != nullptr);
  EXPECT_STREQ(str, "{\"yxyz\":123,\"abcd\":567,\"warningAboveLevel\":10}");
  free(str);
}

} /* namespace testing */
