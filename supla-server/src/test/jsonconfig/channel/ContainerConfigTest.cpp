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

#include <string>

#include "TestHelper.h"
#include "jsonconfig/channel/container_config.h"

namespace testing {
ContainerConfigTest::ContainerConfigTest(void) {}
ContainerConfigTest::~ContainerConfigTest(void) {}

TEST_F(ContainerConfigTest, CanonicalAllSlotsRoundTrip) {
  TChannelConfig_Container raw = {};
  for (unsigned slot = 0; slot < 10; ++slot) {
    raw.SensorInfo[slot].ChannelId = 100000 + slot;
  }
  raw.WarningAboveLevel = 20;
  raw.AlarmBelowLevel = 40;
  raw.MuteAlarmSoundWithoutAdditionalAuth = 1;
  for (unsigned slot = 0; slot < 10; ++slot) {
    raw.SensorInfo[slot].FillLevel = slot * 10;
  }
  container_config writer;
  writer.set_config(&raw);
  char *json = writer.get_user_config();
  ASSERT_NE(nullptr, json);
  EXPECT_EQ(nullptr, strstr(json, "channelNo"));
  EXPECT_EQ(nullptr, strstr(json, "sensorChannelNumbers"));
  container_config reader;
  reader.set_user_config(json);
  free(json);
  TChannelConfig_Container restored = {};
  ASSERT_TRUE(reader.get_config(&restored));
  EXPECT_EQ(0, memcmp(&raw, &restored, sizeof(raw)));
}

TEST_F(ContainerConfigTest, SparseDuplicateSlotsAreNotCompacted) {
  TChannelConfig_Container raw = {};
  raw.SensorInfo[1].ChannelId = 17;
  raw.SensorInfo[9].ChannelId = 17;
  raw.SensorInfo[0].FillLevel = 40;
  raw.SensorInfo[1].FillLevel = 10;
  raw.SensorInfo[9].FillLevel = 90;
  container_config config;
  config.set_config(&raw);
  TChannelConfig_Container restored = {};
  ASSERT_TRUE(config.get_config(&restored));
  EXPECT_EQ(0, memcmp(&raw, &restored, sizeof(raw)));
}

TEST_F(ContainerConfigTest, HistoricalNumbersAreNotBusinessIds) {
  container_config config;
  config.set_user_config(R"({"sensors":[{"channelNo":0,"fillLevel":50}]})");
  TChannelConfig_Container raw = {};
  config.get_config(&raw);
  for (const auto &entry : raw.SensorInfo) {
    EXPECT_EQ(0, entry.ChannelId);
  }
}

TEST_F(ContainerConfigTest, MalformedAndOversizeSlotsFailSafe) {
  container_config config;
  config.set_user_config(
      R"({"sensors":[{"channelId":null},{"channelId":-1},{"channelId":2147483648},{"channelId":1.5},{"channelId":"17"},{"channelId":17,"fillLevel":50}]})");
  TChannelConfig_Container raw = {};
  ASSERT_TRUE(config.get_config(&raw));
  for (unsigned slot = 0; slot < 5; ++slot) {
    EXPECT_EQ(0, raw.SensorInfo[slot].ChannelId);
  }
  EXPECT_EQ(17, raw.SensorInfo[5].ChannelId);
}

TEST_F(ContainerConfigTest, ProtectedRemoteMergeMatrixAndScalar) {
  for (int incomingId : {17, 0, 18, 19}) {
    TChannelConfig_Container raw = {};
    raw.SensorInfo[2].ChannelId = 17;
    container_config authority;
    authority.set_config(&raw);
    raw.SensorInfo[2].ChannelId = incomingId;
    raw.SensorInfo[3].ChannelId = 18;
    raw.WarningAboveLevel = 51;
    raw.SensorInfo[2].FillLevel = 60;
    container_config device;
    device.set_config(&raw);
    device.protect_device_references(
        [](unsigned id) { return id == 0 || id == 18; });
    device.merge(&authority);
    TChannelConfig_Container merged = {};
    ASSERT_TRUE(authority.get_config(&merged));
    EXPECT_EQ(17, merged.SensorInfo[2].ChannelId);
    EXPECT_EQ(18, merged.SensorInfo[3].ChannelId);
    EXPECT_EQ(51, merged.WarningAboveLevel);
    EXPECT_EQ(60, merged.SensorInfo[2].FillLevel);
  }
}

TEST_F(ContainerConfigTest, ProtectedLocalUnsetCannotCreateRemote) {
  for (int oldId : {0, 18}) {
    TChannelConfig_Container raw = {};
    raw.SensorInfo[0].ChannelId = oldId;
    container_config authority;
    authority.set_config(&raw);
    raw.SensorInfo[0].ChannelId = 19;
    container_config device;
    device.set_config(&raw);
    device.protect_device_references(
        [](unsigned id) { return id == 0 || id == 18; });
    device.merge(&authority);
    ASSERT_TRUE(authority.get_config(&raw));
    EXPECT_EQ(oldId, raw.SensorInfo[0].ChannelId);
  }
}

TEST_F(ContainerConfigTest, OversizeArrayDoesNotWritePastProtocolSlots) {
  std::string json = "{\"sensors\":[";
  for (unsigned slot = 0; slot < 10 + 2; ++slot) {
    if (slot) {
      json += ",";
    }
    json += "{\"channelId\":" + std::to_string(slot + 1) + ",\"fillLevel\":50}";
  }
  json += "]}";
  container_config config;
  config.set_user_config(json.c_str());
  struct {
    TChannelConfig_Container raw;
    unsigned int canary;
  } guarded = {};
  guarded.canary = 0x12345678;
  ASSERT_TRUE(config.get_config(&guarded.raw));
  EXPECT_EQ(10, guarded.raw.SensorInfo[9].ChannelId);
  EXPECT_EQ(0x12345678u, guarded.canary);
}

TEST_F(ContainerConfigTest, AllScalarSettingsSurviveCanonicalRoundTrip) {
  TChannelConfig_Container raw = {};
  raw.WarningAboveLevel = 101;
  raw.AlarmAboveLevel = 0;
  raw.WarningBelowLevel = 51;
  raw.AlarmBelowLevel = 100;
  raw.MuteAlarmSoundWithoutAdditionalAuth = 1;
  container_config config;
  config.set_config(&raw);
  TChannelConfig_Container restored = {};
  ASSERT_TRUE(config.get_config(&restored));
  EXPECT_EQ(0, memcmp(&raw, &restored, sizeof(raw)));
}
}  // namespace testing
