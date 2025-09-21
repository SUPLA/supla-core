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

#include "ChannelStateTest.h"

#include "TestHelper.h"
#include "device/channel_state.h"

namespace testing {

TEST_F(ChannelStateTest, ffffffff_rawToJsonAndBack) {
  TDSC_ChannelState raw = {};
  raw.Fields = 0xFFFFFFFF;
  raw.defaultIconField = 1;
  raw.IPv4 = 336242880;
  for (unsigned char a = 0; a < sizeof(raw.MAC); a++) {
    raw.MAC[a] = 0xFA + a;
  }

  raw.BatteryLevel = 4;
  raw.BatteryPowered = 5;
  raw.WiFiRSSI = 6;
  raw.WiFiSignalStrength = 7;
  raw.BridgeNodeOnline = 8;
  raw.BridgeNodeSignalStrength = 9;
  raw.Uptime = 10;
  raw.ConnectionUptime = 11;
  raw.BatteryHealth = 12;
  raw.LastConnectionResetCause = 13;
  raw.LightSourceLifespan = 14;

  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(
      json,
      "{\"switchCycleCount\":1,\"ipv4\":\"192.168.10.20\",\"mac\":\"FA:FB:FC:"
      "FD:FE:FF\",\"batteryLevel\":4,\"batteryPowered\":true,\"wifiRSSI\":6,"
      "\"wifiSignalStrength\":7,\"bridgeNodeOnline\":true,"
      "\"bridgeNodeSignalStrength\":9,\"uptime\":10,\"connectionUptime\":11,"
      "\"batteryHealth\":12,\"lastConnectionResetCause\":13,"
      "\"lightSourceLifespan\":14,\"deviceBatteryLevel\":true}");

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_IPV4 | SUPLA_CHANNELSTATE_FIELD_MAC |
               SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL |
               SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED |
               SUPLA_CHANNELSTATE_FIELD_WIFIRSSI |
               SUPLA_CHANNELSTATE_FIELD_WIFISIGNALSTRENGTH |
               SUPLA_CHANNELSTATE_FIELD_BRIDGENODESIGNALSTRENGTH |
               SUPLA_CHANNELSTATE_FIELD_UPTIME |
               SUPLA_CHANNELSTATE_FIELD_CONNECTIONUPTIME |
               SUPLA_CHANNELSTATE_FIELD_BATTERYHEALTH |
               SUPLA_CHANNELSTATE_FIELD_BRIDGENODEONLINE |
               SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE |
               SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN |
               SUPLA_CHANNELSTATE_FIELD_SWITCHCYCLECOUNT |
               SUPLA_CHANNELSTATE_FIELD_DEVICE_BATTERYLEVEL;

  raw.BatteryPowered = 1;
  raw.BridgeNodeOnline = 1;

  supla_channel_state state2(json);
  EXPECT_EQ(memcmp(&raw, state2.get_state(), sizeof(TDSC_ChannelState)), 0);

  EXPECT_EQ(raw.Fields, state2.get_state()->Fields);
  EXPECT_EQ(raw.defaultIconField, state2.get_state()->defaultIconField);
  EXPECT_EQ(raw.IPv4, state2.get_state()->IPv4);
  for (unsigned char a = 0; a < sizeof(raw.MAC); a++) {
    EXPECT_EQ(raw.MAC[a], state2.get_state()->MAC[a]);
  }
  EXPECT_EQ(raw.BatteryLevel, state2.get_state()->BatteryLevel);
  EXPECT_EQ(raw.BatteryPowered, state2.get_state()->BatteryPowered);
  EXPECT_EQ(raw.WiFiRSSI, state2.get_state()->WiFiRSSI);
  EXPECT_EQ(raw.WiFiSignalStrength, state2.get_state()->WiFiSignalStrength);
  EXPECT_EQ(raw.BridgeNodeOnline, state2.get_state()->BridgeNodeOnline);
  EXPECT_EQ(raw.BridgeNodeSignalStrength,
            state2.get_state()->BridgeNodeSignalStrength);
  EXPECT_EQ(raw.Uptime, state2.get_state()->Uptime);
  EXPECT_EQ(raw.ConnectionUptime, state2.get_state()->ConnectionUptime);
  EXPECT_EQ(raw.BatteryHealth, state2.get_state()->BatteryHealth);
  EXPECT_EQ(raw.LastConnectionResetCause,
            state2.get_state()->LastConnectionResetCause);
  EXPECT_EQ(raw.LightSourceLifespan, state2.get_state()->LightSourceLifespan);
  EXPECT_EQ(raw.LightSourceLifespanLeft,
            state2.get_state()->LightSourceLifespanLeft);

  free(json);
}

TEST_F(ChannelStateTest, getJSON_batteryPowered_false) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
  raw.BatteryPowered = 0;
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json, "{\"defaultIconField\":0,\"batteryPowered\":false}");

  free(json);
}

TEST_F(ChannelStateTest, getJSON_batteryPowered_true) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
  raw.BatteryPowered = 1;
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json, "{\"defaultIconField\":0,\"batteryPowered\":true}");

  free(json);
}

TEST_F(ChannelStateTest, getJSON_bridgeNodeOnline_false) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BRIDGENODEONLINE;
  raw.BatteryPowered = 0;
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json, "{\"defaultIconField\":0,\"bridgeNodeOnline\":false}");

  free(json);
}

TEST_F(ChannelStateTest, getJSON_bridgeNodeOnline_true) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BRIDGENODEONLINE;
  raw.BridgeNodeOnline = 1;
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json, "{\"defaultIconField\":0,\"bridgeNodeOnline\":true}");

  free(json);
}

TEST_F(ChannelStateTest, getJSON_lightSourceOperatingTime) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME;
  raw.LightSourceOperatingTime = 55;
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json,
               "{\"defaultIconField\":0,\"lightSourceOperatingTime\":55}");

  free(json);
}

TEST_F(ChannelStateTest, getJSON_operatingTime) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME;
  raw.OperatingTime = 35;
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json, "{\"defaultIconField\":0,\"operatingTime\":35}");

  free(json);
}

TEST_F(ChannelStateTest, lightSourceLifespanToJsonAndBack) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN;
  raw.LightSourceLifespan = 14;
  raw.LightSourceLifespanLeft = 15;

  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json,
               "{\"defaultIconField\":0,\"lightSourceLifespan\":14,"
               "\"lightSourceLifespanLeft\":15}");

  supla_channel_state state2(json);

  free(json);

  EXPECT_EQ(state2.get_state()->Fields, raw.Fields);
  EXPECT_EQ(state2.get_state()->LightSourceLifespan, raw.LightSourceLifespan);
  EXPECT_EQ(state2.get_state()->LightSourceLifespanLeft,
            raw.LightSourceLifespanLeft);
}

TEST_F(ChannelStateTest,
       lightSourceLifespanWithLightSourceOperatingTimeToJsonAndBack) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN |
               SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME;
  raw.LightSourceLifespan = 14;
  raw.LightSourceOperatingTime = 25;

  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json,
               "{\"defaultIconField\":0,\"lightSourceLifespan\":14,"
               "\"lightSourceOperatingTime\":25}");

  supla_channel_state state2(json);

  free(json);

  EXPECT_EQ(state2.get_state()->Fields, raw.Fields);
  EXPECT_EQ(state2.get_state()->LightSourceLifespan, raw.LightSourceLifespan);
  EXPECT_EQ(state2.get_state()->LightSourceOperatingTime,
            raw.LightSourceOperatingTime);
}

TEST_F(ChannelStateTest, lightSourceLifespanWithOperatingTimeToJsonAndBack) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN |
               SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME;
  raw.LightSourceLifespan = 14;
  raw.OperatingTime = 25;

  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json,
               "{\"defaultIconField\":0,\"lightSourceLifespan\":14,"
               "\"operatingTime\":25}");

  supla_channel_state state2(json);

  free(json);

  EXPECT_EQ(state2.get_state()->Fields, raw.Fields);
  EXPECT_EQ(state2.get_state()->LightSourceLifespan, raw.LightSourceLifespan);
  EXPECT_EQ(state2.get_state()->OperatingTime, raw.OperatingTime);
}

TEST_F(ChannelStateTest, lastConnectionResetCauseStringToJsonAndBack) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE;
  raw.LastConnectionResetCause =
      SUPLA_LASTCONNECTIONRESETCAUSE_ACTIVITY_TIMEOUT;

  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json,
               "{\"defaultIconField\":0,\"lastConnectionResetCause\":"
               "\"ACTIVITY_TIMEOUT\"}");

  supla_channel_state state2(json);

  free(json);

  EXPECT_EQ(state2.get_state()->Fields, raw.Fields);
  EXPECT_EQ(state2.get_state()->LastConnectionResetCause,
            raw.LastConnectionResetCause);
}

TEST_F(ChannelStateTest, lastConnectionResetCauseIntToJsonAndBack) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE;
  raw.LastConnectionResetCause = 200;

  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json,
               "{\"defaultIconField\":0,\"lastConnectionResetCause\":200}");

  supla_channel_state state2(json);

  free(json);

  EXPECT_EQ(state2.get_state()->Fields, raw.Fields);
  EXPECT_EQ(state2.get_state()->LastConnectionResetCause,
            raw.LastConnectionResetCause);
}

TEST_F(ChannelStateTest, mergeIfNeeded_batteryLevel) {
  TDSC_ChannelState raw = {};
  TDSC_ChannelState old_raw = {};

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL;
  raw.BatteryLevel = 33;

  supla_channel_state state(&raw);

  {
    supla_channel_state old_state(&old_raw);
    state.merge_old_if_needed(&old_state);
  }

  EXPECT_EQ(state.get_state()->Fields, SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL);
  EXPECT_EQ(state.get_state()->BatteryLevel, 33);

  old_raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL;
  old_raw.BatteryLevel = 50;

  {
    supla_channel_state old_state(&old_raw);
    state.merge_old_if_needed(&old_state);
  }

  EXPECT_EQ(state.get_state()->Fields, SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL);
  EXPECT_EQ(state.get_state()->BatteryLevel, 33);

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_IPV4;

  {
    state = supla_channel_state(&raw);
    supla_channel_state old_state(&old_raw);
    state.merge_old_if_needed(&old_state);
  }

  EXPECT_EQ(
      state.get_state()->Fields,
      SUPLA_CHANNELSTATE_FIELD_IPV4 | SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL);
  EXPECT_EQ(state.get_state()->BatteryLevel, 50);
}

TEST_F(ChannelStateTest, mergeIfNeeded_batteryPowered) {
  TDSC_ChannelState raw = {};
  TDSC_ChannelState old_raw = {};

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
  raw.BatteryPowered = 1;

  supla_channel_state state(&raw);

  {
    supla_channel_state old_state(&old_raw);
    state.merge_old_if_needed(&old_state);
  }

  EXPECT_EQ(state.get_state()->Fields, SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED);
  EXPECT_EQ(state.get_state()->BatteryPowered, 1);

  old_raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;

  {
    supla_channel_state old_state(&old_raw);
    state.merge_old_if_needed(&old_state);
  }

  EXPECT_EQ(state.get_state()->Fields, SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED);
  EXPECT_EQ(state.get_state()->BatteryPowered, 1);

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_MAC;

  {
    state = supla_channel_state(&raw);
    supla_channel_state old_state(&old_raw);
    state.merge_old_if_needed(&old_state);
  }

  EXPECT_EQ(
      state.get_state()->Fields,
      SUPLA_CHANNELSTATE_FIELD_MAC | SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED);
  EXPECT_EQ(state.get_state()->BatteryPowered, 0);
}

TEST_F(ChannelStateTest, equalFields) {
  TDSC_ChannelState raw1 = {};

  TestHelper::randomize((char *)&raw1, sizeof(TDSC_ChannelState));
  TDSC_ChannelState raw2 = raw1;
  raw1.ReceiverID++;
  raw1.ChannelID++;
  raw1.EmptySpace[0]++;

  supla_channel_state state1(&raw1);

  supla_channel_state state2(&raw2);
  EXPECT_TRUE(state1.equal_fields(&state2));

  unsigned char skip_leading_bytes =
      sizeof(raw1.ReceiverID) + sizeof(raw1.ChannelID);
  unsigned char skip_trailing_bytes = sizeof(raw1.EmptySpace);

  for (unsigned int a = 0; a < sizeof(TDSC_ChannelState) -
                                   (skip_leading_bytes + skip_trailing_bytes);
       a++) {
    TDSC_ChannelState raw3 = raw1;
    ((char *)&raw3)[skip_leading_bytes + a]++;
    supla_channel_state state3(&raw3);

    EXPECT_FALSE(state1.equal_fields(&state3));
  }
}

TEST_F(ChannelStateTest, getVbtValue) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL |
               SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
  supla_channel_state state(&raw);

  double value = 0;
  EXPECT_FALSE(state.get_vbt_value(var_name_none, &value));

  EXPECT_TRUE(state.get_vbt_value(var_name_battery_level, &value));
  EXPECT_EQ(value, 0);

  EXPECT_TRUE(state.get_vbt_value(var_name_battery_powered, &value));
  EXPECT_EQ(value, 0);

  raw.BatteryLevel = 22;
  raw.BatteryPowered = 1;

  state = supla_channel_state(&raw);

  EXPECT_TRUE(state.get_vbt_value(var_name_battery_level, &value));
  EXPECT_EQ(value, 22);

  EXPECT_TRUE(state.get_vbt_value(var_name_battery_powered, &value));
  EXPECT_EQ(value, 1);
}

TEST_F(ChannelStateTest, getJSON_deviceBatteryLevel_notSet) {
  TDSC_ChannelState raw = {};
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json, "{\"defaultIconField\":0}");

  free(json);
}

TEST_F(ChannelStateTest, getJSON_deviceBatteryLevel_set) {
  TDSC_ChannelState raw = {};
  raw.Fields = SUPLA_CHANNELSTATE_FIELD_DEVICE_BATTERYLEVEL;
  supla_channel_state state(&raw);
  char *json = state.get_json();
  ASSERT_NE(json, nullptr);

  EXPECT_STREQ(json, "{\"defaultIconField\":0,\"deviceBatteryLevel\":true}");

  free(json);
}

} /* namespace testing */
