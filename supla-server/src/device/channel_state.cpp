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

#include "channel_state.h"

#include <arpa/inet.h>
#include <string.h>

#include "json/cJSON.h"

using std::map;
using std::string;

#define JSON_FIELD_DEFAULT_ICON_FIELD 1
#define JSON_FIELD_SWITCH_CYCLE_COUNT 2
#define JSON_FIELD_IPV4 3
#define JSON_FIELD_MAC 4
#define JSON_FIELD_BATTERY_LEVEL 5
#define JSON_FIELD_BATTERY_POWERED 6
#define JSON_FIELD_WIFI_RSSI 7
#define JSON_FIELD_WIFI_SIGNAL_STRENGTH 8
#define JSON_FIELD_BRIDGE_NODE_ONLINE 9
#define JSON_FIELD_BRIDGE_NODE_SIGNAL_STRENGTH 10
#define JSON_FIELD_UPTIME 11
#define JSON_FIELD_CONNECTION_UPTIME 12
#define JSON_FIELD_BATTERY_HEALTH 13
#define JSON_FIELD_LAST_CONNECTION_RESET_CAUSE 14
#define JSON_FIELD_LIGHT_SOURCE_LIFESPAN 15
#define JSON_FIELD_LIGHT_SOURCE_LIFESPAN_LEFT 16
#define JSON_FIELD_LIGHT_SOURCE_OPERATING_TIME 17
#define JSON_FIELD_OPERATING_TIME 18

const map<unsigned _supla_int16_t, string> supla_channel_state::field_map = {
    {JSON_FIELD_DEFAULT_ICON_FIELD, "defaultIconField"},
    {JSON_FIELD_SWITCH_CYCLE_COUNT, "switchCycleCount"},
    {JSON_FIELD_IPV4, "ipv4"},
    {JSON_FIELD_MAC, "mac"},
    {JSON_FIELD_BATTERY_LEVEL, "batteryLevel"},
    {JSON_FIELD_BATTERY_POWERED, "batteryPowered"},
    {JSON_FIELD_WIFI_RSSI, "wifiRSSI"},
    {JSON_FIELD_WIFI_SIGNAL_STRENGTH, "wifiSignalStrength"},
    {JSON_FIELD_BRIDGE_NODE_ONLINE, "bridgeNodeOnline"},
    {JSON_FIELD_BRIDGE_NODE_SIGNAL_STRENGTH, "bridgeNodeSignalStrength"},
    {JSON_FIELD_UPTIME, "uptime"},
    {JSON_FIELD_CONNECTION_UPTIME, "connectionUptime"},
    {JSON_FIELD_BATTERY_HEALTH, "batteryHealth"},
    {JSON_FIELD_LAST_CONNECTION_RESET_CAUSE, "lastConnectionResetCause"},
    {JSON_FIELD_LIGHT_SOURCE_LIFESPAN, "lightSourceLifespan"},
    {JSON_FIELD_LIGHT_SOURCE_LIFESPAN_LEFT, "lightSourceLifespanLeft"},
    {JSON_FIELD_LIGHT_SOURCE_OPERATING_TIME, "lightSourceOperatingTime"},
    {JSON_FIELD_OPERATING_TIME, "operatingTime"},
};

supla_channel_state::supla_channel_state(void) { state = {}; }

supla_channel_state::supla_channel_state(TDSC_ChannelState *state)
    : supla_json_helper() {
  this->state = *state;
}

supla_channel_state::supla_channel_state(const char *json)
    : supla_json_helper() {
  state = {};
  apply_json(json);
}

supla_channel_state::~supla_channel_state() {}

bool supla_channel_state::equal_fields(supla_channel_state *state) {
  if (!state) {
    return false;
  }

  unsigned char skip_leading_bytes =
      sizeof(this->state.ReceiverID) + sizeof(this->state.ChannelID);
  unsigned char skip_trailing_bytes = sizeof(this->state.EmptySpace);

  return memcmp(&((char *)&this->state)[skip_leading_bytes],
                &((char *)&state->state)[skip_leading_bytes],
                sizeof(TDSC_ChannelState) - skip_leading_bytes -
                    skip_trailing_bytes) == 0;
}

const string supla_channel_state::reset_cause_to_string(
    unsigned char reset_cause) {
  switch (reset_cause) {
    case SUPLA_LASTCONNECTIONRESETCAUSE_UNKNOWN:
      return "UNKNOWN";
    case SUPLA_LASTCONNECTIONRESETCAUSE_ACTIVITY_TIMEOUT:
      return "ACTIVITY_TIMEOUT";
    case SUPLA_LASTCONNECTIONRESETCAUSE_WIFI_CONNECTION_LOST:
      return "WIFI_CONNECTION_LOST";
    case SUPLA_LASTCONNECTIONRESETCAUSE_SERVER_CONNECTION_LOST:
      return "SERVER_CONNECTION_LOST";
  }

  return "";
}

unsigned char supla_channel_state::string_to_reset_cause(
    const std::string &reset_cause) {
  if (reset_cause == "UNKNOWN") {
    return SUPLA_LASTCONNECTIONRESETCAUSE_UNKNOWN;
  } else if (reset_cause == "ACTIVITY_TIMEOUT") {
    return SUPLA_LASTCONNECTIONRESETCAUSE_ACTIVITY_TIMEOUT;
  } else if (reset_cause == "WIFI_CONNECTION_LOSTT") {
    return SUPLA_LASTCONNECTIONRESETCAUSE_WIFI_CONNECTION_LOST;
  } else if (reset_cause == "SERVER_CONNECTION_LOST") {
    return SUPLA_LASTCONNECTIONRESETCAUSE_SERVER_CONNECTION_LOST;
  }
  return 0;
}

void supla_channel_state::apply_json(const char *json) {
  if (!json) {
    return;
  }

  cJSON *root = cJSON_Parse(json);

  if (!root) {
    return;
  }

  double dbl_val = 0;

  state = {};

  if (get_double(root, field_map.at(JSON_FIELD_SWITCH_CYCLE_COUNT).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_SWITCHCYCLECOUNT;
    state.SwitchCycleCount = dbl_val;
  } else if (get_double(root,
                        field_map.at(JSON_FIELD_DEFAULT_ICON_FIELD).c_str(),
                        &dbl_val)) {
    state.defaultIconField = dbl_val;
  }

  string str_val;

  if (get_string(root, field_map.at(JSON_FIELD_IPV4).c_str(), &str_val)) {
    struct in_addr addr = {};
    if (inet_pton(AF_INET, str_val.c_str(), &addr) == 1) {
      state.Fields |= SUPLA_CHANNELSTATE_FIELD_IPV4;
      state.IPv4 = ntohl(addr.s_addr);
    }
  }

  if (get_string(root, field_map.at(JSON_FIELD_MAC).c_str(), &str_val) &&
      str_val.size() == 17) {
    int mac[6];
    if (sscanf(str_val.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2],
               &mac[3], &mac[4], &mac[5]) == 6) {
      for (unsigned char i = 0; i < 6; i++) {
        state.MAC[i] = (unsigned char)mac[i];
      }

      state.Fields |= SUPLA_CHANNELSTATE_FIELD_MAC;
    }
  }

  if (get_double(root, field_map.at(JSON_FIELD_BATTERY_LEVEL).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL;
    state.BatteryLevel = dbl_val;
  }

  bool bool_val = false;

  if (get_bool(root, field_map.at(JSON_FIELD_BATTERY_POWERED).c_str(),
               &bool_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
    state.BatteryPowered = bool_val ? 1 : 0;
  }

  if (get_double(root, field_map.at(JSON_FIELD_WIFI_RSSI).c_str(), &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_WIFIRSSI;
    state.WiFiRSSI = dbl_val;
  }

  if (get_double(root, field_map.at(JSON_FIELD_WIFI_SIGNAL_STRENGTH).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_WIFISIGNALSTRENGTH;
    state.WiFiSignalStrength = dbl_val;
  }

  if (get_bool(root, field_map.at(JSON_FIELD_BRIDGE_NODE_ONLINE).c_str(),
               &bool_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_BRIDGENODEONLINE;
    state.BridgeNodeOnline = bool_val ? 1 : 0;
  }

  if (get_double(root,
                 field_map.at(JSON_FIELD_BRIDGE_NODE_SIGNAL_STRENGTH).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_BRIDGENODESIGNALSTRENGTH;
    state.BridgeNodeSignalStrength = dbl_val;
  }

  if (get_double(root, field_map.at(JSON_FIELD_UPTIME).c_str(), &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_UPTIME;
    state.Uptime = dbl_val;
  }

  if (get_double(root, field_map.at(JSON_FIELD_CONNECTION_UPTIME).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_CONNECTIONUPTIME;
    state.ConnectionUptime = dbl_val;
  }

  if (get_double(root, field_map.at(JSON_FIELD_BATTERY_HEALTH).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_BATTERYHEALTH;
    state.BatteryHealth = dbl_val;
  }

  if (get_double(root,
                 field_map.at(JSON_FIELD_LAST_CONNECTION_RESET_CAUSE).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE;
    state.LastConnectionResetCause = dbl_val;
  } else if (get_string(
                 root,
                 field_map.at(JSON_FIELD_LAST_CONNECTION_RESET_CAUSE).c_str(),
                 &str_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE;
    state.LastConnectionResetCause = string_to_reset_cause(str_val);
  }

  if (get_double(root, field_map.at(JSON_FIELD_LIGHT_SOURCE_LIFESPAN).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN;
    state.LightSourceLifespan = dbl_val;
  }

  if (get_double(root,
                 field_map.at(JSON_FIELD_LIGHT_SOURCE_LIFESPAN_LEFT).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN;
    state.LightSourceLifespanLeft = dbl_val;
  } else if (get_double(
                 root,
                 field_map.at(JSON_FIELD_LIGHT_SOURCE_OPERATING_TIME).c_str(),
                 &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME;
    state.LightSourceOperatingTime = dbl_val;
  } else if (get_double(root, field_map.at(JSON_FIELD_OPERATING_TIME).c_str(),
                        &dbl_val)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME;
    state.OperatingTime = dbl_val;
  }

  cJSON_Delete(root);
}

char *supla_channel_state::get_json(void) {
  cJSON *root = cJSON_CreateObject();

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_SWITCHCYCLECOUNT) {
    set_item_value(root, field_map.at(JSON_FIELD_SWITCH_CYCLE_COUNT).c_str(),
                   cJSON_Number, true, nullptr, nullptr,
                   state.SwitchCycleCount);
  } else {
    set_item_value(root, field_map.at(JSON_FIELD_DEFAULT_ICON_FIELD).c_str(),
                   cJSON_Number, true, nullptr, nullptr,
                   state.defaultIconField);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_IPV4) {
    struct in_addr addr;
    addr.s_addr = htonl(state.IPv4);

    char ipv4[INET_ADDRSTRLEN] = {};

    if (inet_ntop(AF_INET, &addr, ipv4, INET_ADDRSTRLEN) == nullptr) {
      ipv4[0] = 0;
    }

    set_item_value(root, field_map.at(JSON_FIELD_IPV4).c_str(), cJSON_String,
                   true, nullptr, ipv4, 0);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_MAC) {
    char mac[18] = {};

    snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
             (unsigned char)state.MAC[0], (unsigned char)state.MAC[1],
             (unsigned char)state.MAC[2], (unsigned char)state.MAC[3],
             (unsigned char)state.MAC[4], (unsigned char)state.MAC[5]);

    set_item_value(root, field_map.at(JSON_FIELD_MAC).c_str(), cJSON_String,
                   true, nullptr, mac, 0);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL) {
    set_item_value(root, field_map.at(JSON_FIELD_BATTERY_LEVEL).c_str(),
                   cJSON_Number, true, nullptr, nullptr, state.BatteryLevel);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED) {
    set_item_value(root, field_map.at(JSON_FIELD_BATTERY_POWERED).c_str(),
                   state.BatteryPowered ? cJSON_True : cJSON_False, true,
                   nullptr, nullptr, 0);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_WIFIRSSI) {
    set_item_value(root, field_map.at(JSON_FIELD_WIFI_RSSI).c_str(),
                   cJSON_Number, true, nullptr, nullptr, state.WiFiRSSI);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_WIFISIGNALSTRENGTH) {
    set_item_value(root, field_map.at(JSON_FIELD_WIFI_SIGNAL_STRENGTH).c_str(),
                   cJSON_Number, true, nullptr, nullptr,
                   state.WiFiSignalStrength);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BRIDGENODEONLINE) {
    set_item_value(root, field_map.at(JSON_FIELD_BRIDGE_NODE_ONLINE).c_str(),
                   state.BridgeNodeOnline ? cJSON_True : cJSON_False, true,
                   nullptr, nullptr, 0);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BRIDGENODESIGNALSTRENGTH) {
    set_item_value(
        root, field_map.at(JSON_FIELD_BRIDGE_NODE_SIGNAL_STRENGTH).c_str(),
        cJSON_Number, true, nullptr, nullptr, state.BridgeNodeSignalStrength);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_UPTIME) {
    set_item_value(root, field_map.at(JSON_FIELD_UPTIME).c_str(), cJSON_Number,
                   true, nullptr, nullptr, state.Uptime);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_CONNECTIONUPTIME) {
    set_item_value(root, field_map.at(JSON_FIELD_CONNECTION_UPTIME).c_str(),
                   cJSON_Number, true, nullptr, nullptr,
                   state.ConnectionUptime);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYHEALTH) {
    set_item_value(root, field_map.at(JSON_FIELD_BATTERY_HEALTH).c_str(),
                   cJSON_Number, true, nullptr, nullptr, state.BatteryHealth);
  }

  if (state.Fields & SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE) {
    if (state.LastConnectionResetCause >
        SUPLA_LASTCONNECTIONRESETCAUSE_SERVER_CONNECTION_LOST) {
      set_item_value(
          root, field_map.at(JSON_FIELD_LAST_CONNECTION_RESET_CAUSE).c_str(),
          cJSON_Number, true, nullptr, nullptr, state.LastConnectionResetCause);
    } else {
      set_item_value(
          root, field_map.at(JSON_FIELD_LAST_CONNECTION_RESET_CAUSE).c_str(),
          cJSON_String, true, nullptr,
          reset_cause_to_string(state.LastConnectionResetCause).c_str(), 0);
    }
  }

  if ((state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN)) {
    set_item_value(root, field_map.at(JSON_FIELD_LIGHT_SOURCE_LIFESPAN).c_str(),
                   cJSON_Number, true, nullptr, nullptr,
                   state.LightSourceLifespan);
  }

  if ((state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN) &&
      !(state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME) &&
      !(state.Fields & SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME)) {
    set_item_value(
        root, field_map.at(JSON_FIELD_LIGHT_SOURCE_LIFESPAN_LEFT).c_str(),
        cJSON_Number, true, nullptr, nullptr, state.LightSourceLifespanLeft);
  }

  if ((state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME) &&
      !(state.Fields & SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME)) {
    set_item_value(
        root, field_map.at(JSON_FIELD_LIGHT_SOURCE_OPERATING_TIME).c_str(),
        cJSON_Number, true, nullptr, nullptr, state.LightSourceOperatingTime);
  }

  if (!(state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME) &&
      (state.Fields & SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME)) {
    set_item_value(root, field_map.at(JSON_FIELD_OPERATING_TIME).c_str(),
                   cJSON_Number, true, nullptr, nullptr, state.OperatingTime);
  }

  char *result = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  return result;
}

const TDSC_ChannelState *supla_channel_state::get_state(void) { return &state; }

void supla_channel_state::merge_old_if_needed(supla_channel_state *old) {
  if (!(state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL) &&
      (old->state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL;
    state.BatteryLevel = old->state.BatteryLevel;
  }

  if (!(state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED) &&
      (old->state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED)) {
    state.Fields |= SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
    state.BatteryPowered = old->state.BatteryPowered;
  }
}
