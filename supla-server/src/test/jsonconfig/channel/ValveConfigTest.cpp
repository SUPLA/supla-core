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

#include <string>

#include "TestHelper.h"
#include "jsonconfig/channel/valve_config.h"

namespace testing {
ValveConfigTest::ValveConfigTest(void) {}
ValveConfigTest::~ValveConfigTest(void) {}

TEST_F(ValveConfigTest, CanonicalAllSlotsRoundTrip) {
  TChannelConfig_Valve raw = {};
  for (unsigned slot = 0; slot < 20; ++slot) {
    raw.SensorInfo[slot].ChannelId = 100000 + slot;
  }
  raw.CloseValveOnFloodType = SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE;
  valve_config writer;
  writer.set_config(&raw);
  char *json = writer.get_user_config();
  ASSERT_NE(nullptr, json);
  EXPECT_EQ(nullptr, strstr(json, "channelNo"));
  EXPECT_EQ(nullptr, strstr(json, "sensorChannelNumbers"));
  valve_config reader;
  reader.set_user_config(json);
  free(json);
  TChannelConfig_Valve restored = {};
  ASSERT_TRUE(reader.get_config(&restored));
  EXPECT_EQ(0, memcmp(&raw, &restored, sizeof(raw)));
}

TEST_F(ValveConfigTest, SparseDuplicateSlotsAreNotCompacted) {
  TChannelConfig_Valve raw = {};
  raw.SensorInfo[1].ChannelId = 17;
  raw.SensorInfo[19].ChannelId = 17;
  valve_config config;
  config.set_config(&raw);
  TChannelConfig_Valve restored = {};
  ASSERT_TRUE(config.get_config(&restored));
  EXPECT_EQ(0, memcmp(&raw, &restored, sizeof(raw)));
}

TEST_F(ValveConfigTest, HistoricalNumbersAreNotBusinessIds) {
  valve_config config;
  config.set_user_config(R"({"sensorChannelNumbers":[0,1,2]})");
  TChannelConfig_Valve raw = {};
  config.get_config(&raw);
  for (const auto &entry : raw.SensorInfo) {
    EXPECT_EQ(0, entry.ChannelId);
  }
}

TEST_F(ValveConfigTest, MalformedAndOversizeSlotsFailSafe) {
  valve_config config;
  config.set_user_config(
      R"({"floodSensorChannelIds":[null,-1,2147483648,1.5,"17",17]})");
  TChannelConfig_Valve raw = {};
  ASSERT_TRUE(config.get_config(&raw));
  for (unsigned slot = 0; slot < 5; ++slot) {
    EXPECT_EQ(0, raw.SensorInfo[slot].ChannelId);
  }
  EXPECT_EQ(17, raw.SensorInfo[5].ChannelId);
}

TEST_F(ValveConfigTest, ProtectedRemoteMergeMatrixAndScalar) {
  for (int incomingId : {17, 0, 18, 19}) {
    TChannelConfig_Valve raw = {};
    raw.SensorInfo[2].ChannelId = 17;
    valve_config authority;
    authority.set_config(&raw);
    raw.SensorInfo[2].ChannelId = incomingId;
    raw.SensorInfo[3].ChannelId = 18;
    raw.CloseValveOnFloodType = SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE;
    valve_config device;
    device.set_config(&raw);
    device.protect_device_references(
        [](unsigned id) { return id == 0 || id == 18; });
    device.merge(&authority);
    TChannelConfig_Valve merged = {};
    ASSERT_TRUE(authority.get_config(&merged));
    EXPECT_EQ(17, merged.SensorInfo[2].ChannelId);
    EXPECT_EQ(18, merged.SensorInfo[3].ChannelId);
    EXPECT_EQ(SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE,
              merged.CloseValveOnFloodType);
  }
}

TEST_F(ValveConfigTest, ProtectedLocalUnsetCannotCreateRemote) {
  for (int oldId : {0, 18}) {
    TChannelConfig_Valve raw = {};
    raw.SensorInfo[0].ChannelId = oldId;
    valve_config authority;
    authority.set_config(&raw);
    raw.SensorInfo[0].ChannelId = 19;
    valve_config device;
    device.set_config(&raw);
    device.protect_device_references(
        [](unsigned id) { return id == 0 || id == 18; });
    device.merge(&authority);
    ASSERT_TRUE(authority.get_config(&raw));
    EXPECT_EQ(oldId, raw.SensorInfo[0].ChannelId);
  }
}

TEST_F(ValveConfigTest, OversizeArrayDoesNotWritePastProtocolSlots) {
  std::string json = "{\"floodSensorChannelIds\":[";
  for (unsigned slot = 0; slot < 20 + 2; ++slot) {
    if (slot) {
      json += ",";
    }
    json += std::to_string(slot + 1);
  }
  json += "]}";
  valve_config config;
  config.set_user_config(json.c_str());
  struct {
    TChannelConfig_Valve raw;
    unsigned int canary;
  } guarded = {};
  guarded.canary = 0x12345678;
  ASSERT_TRUE(config.get_config(&guarded.raw));
  EXPECT_EQ(20, guarded.raw.SensorInfo[19].ChannelId);
  EXPECT_EQ(0x12345678u, guarded.canary);
}

TEST_F(ValveConfigTest, EveryFloodModeSurvivesCanonicalRoundTrip) {
  for (auto mode : {SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_NONE,
                    SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ALWAYS,
                    SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE}) {
    TChannelConfig_Valve raw = {};
    raw.CloseValveOnFloodType = mode;
    valve_config config;
    config.set_config(&raw);
    TChannelConfig_Valve restored = {};
    config.get_config(&restored);
    EXPECT_EQ(mode, restored.CloseValveOnFloodType);
  }
}
}  // namespace testing
