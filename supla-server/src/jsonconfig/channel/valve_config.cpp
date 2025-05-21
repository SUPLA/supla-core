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

#include "valve_config.h"

#include "log.h"

using std::map;
using std::string;

#define FIELD_SENSORS 1
#define FIELD_CLOSE_ON_FLOOD_TYPE 2

const map<unsigned _supla_int16_t, string> valve_config::field_map = {
    {FIELD_SENSORS, "sensorChannelNumbers"},
    {FIELD_CLOSE_ON_FLOOD_TYPE, "closeValveOnFloodType"}};

valve_config::valve_config(supla_json_config *root) : supla_json_config(root) {}

valve_config::valve_config(void) : supla_json_config() {}

string valve_config::close_on_flood_type_to_string(unsigned char type) {
  switch (type) {
    case SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ALWAYS:
      return "ALWAYS";
    case SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE:
      return "ON_CHANGE";
  }

  return "NONE";
}

unsigned char valve_config::string_to_close_on_flood_type(
    const std::string &type) {
  if (type == "ALWAYS") {
    return SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ALWAYS;
  }
  if (type == "ON_CHANGE") {
    return SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_ON_CHANGE;
  }

  return SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_NONE;
}

void valve_config::set_config(TChannelConfig_Valve *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  cJSON *sensors = cJSON_CreateArray();

  for (size_t a = 0; a < sizeof(config->SensorInfo) / sizeof(TValve_SensorInfo);
       a++) {
    if (config->SensorInfo[a].IsSet) {
      bool found = false;

      for (size_t b = 0; b < a; b++) {
        if (config->SensorInfo[b].IsSet &&
            config->SensorInfo[b].ChannelNo ==
                config->SensorInfo[a].ChannelNo) {
          found = true;
          break;
        }
      }

      if (!found) {
        cJSON_AddItemToArray(
            sensors, cJSON_CreateNumber(config->SensorInfo[a].ChannelNo));
      }
    }
  }

  set_item_value(user_root, field_map.at(FIELD_SENSORS).c_str(), cJSON_Object,
                 true, sensors, nullptr, 0);

  if (config->CloseValveOnFloodType == SUPLA_VALVE_CLOSE_ON_FLOOD_TYPE_NONE) {
    cJSON_DeleteItemFromObject(user_root,
                               field_map.at(FIELD_CLOSE_ON_FLOOD_TYPE).c_str());
  } else {
    set_item_value(
        user_root, field_map.at(FIELD_CLOSE_ON_FLOOD_TYPE).c_str(),
        cJSON_String, true, nullptr,
        close_on_flood_type_to_string(config->CloseValveOnFloodType).c_str(),
        0);
  }
}

bool valve_config::get_config(TChannelConfig_Valve *config) {
  bool result = false;

  *config = {};

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  unsigned char n = 0;

  cJSON *sensors =
      cJSON_GetObjectItem(user_root, field_map.at(FIELD_SENSORS).c_str());
  if (sensors) {
    result = true;
    for (int a = 0; a < cJSON_GetArraySize(sensors); a++) {
      cJSON *item = cJSON_GetArrayItem(sensors, a);

      if (item && cJSON_IsNumber(item)) {
        unsigned char number = cJSON_GetNumberValue(item);
        bool found = false;

        for (size_t b = 0;
             b < sizeof(config->SensorInfo) / sizeof(TValve_SensorInfo); b++) {
          if (config->SensorInfo[b].IsSet &&
              config->SensorInfo[b].ChannelNo == number) {
            found = true;
            break;
          }
        }

        if (!found) {
          config->SensorInfo[n].IsSet = 1;
          config->SensorInfo[n].ChannelNo = cJSON_GetNumberValue(item);
          n++;
          if (n >= sizeof(config->SensorInfo) / sizeof(TValve_SensorInfo)) {
            break;
          }
        }
      }
    }
  }

  string str_value;

  if (get_string(user_root, field_map.at(FIELD_CLOSE_ON_FLOOD_TYPE).c_str(),
                 &str_value)) {
    result = true;
  }

  config->CloseValveOnFloodType = string_to_close_on_flood_type(str_value);

  return result;
}

void valve_config::merge(supla_json_config *_dst) {
  valve_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           true);
}
