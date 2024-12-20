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

#include "container_config.h"

#include "log.h"

using std::map;
using std::string;

#define FIELD_WARNING_ABOVE_LEVEL 1
#define FIELD_ALARM_ABOVE_LEVEL 2
#define FIELD_WARNING_BELOW_LEVEL 3
#define FIELD_ALARM_BELOW_LEVEL 4
#define FIELD_MUTE_ALARM_SOUND_WITHOUT_ADDITIONAL_AUTH 5
#define FIELD_SENSORS 6

#define SENSOR_FIELD_FILL_LEVEL 1

const map<unsigned _supla_int16_t, string> container_config::field_map = {
    {FIELD_WARNING_ABOVE_LEVEL, "warningAboveLevel"},
    {FIELD_ALARM_ABOVE_LEVEL, "alarmAboveLevel"},
    {FIELD_WARNING_BELOW_LEVEL, "warningBelowLevel"},
    {FIELD_ALARM_BELOW_LEVEL, "alarmBelowLevel"},
    {FIELD_MUTE_ALARM_SOUND_WITHOUT_ADDITIONAL_AUTH,
     "muteAlarmSoundWithoutAdditionalAuth"},
    {FIELD_SENSORS, "sensors"},
};

const std::map<unsigned _supla_int16_t, std::string>
    container_config::sensor_field_map = {
        {SENSOR_FIELD_FILL_LEVEL, "fillLevel"},
};

container_config::container_config(supla_json_config *root)
    : supla_json_config(root) {}

container_config::container_config(void) : supla_json_config() {}

void container_config::set_level(cJSON *parent, int field,
                                 unsigned char level) {
  set_item_value(parent, field_map.at(field).c_str(),
                 level > 0 && level <= 101 ? cJSON_Number : cJSON_NULL, true,
                 nullptr, nullptr, level - 1);
}

bool container_config::get_level(cJSON *parent, int field,
                                 unsigned char *level) {
  *level = 0;
  double dbl_value = 0;
  if (get_double(parent, field_map.at(field).c_str(), &dbl_value)) {
    if (dbl_value >= 0 && dbl_value <= 100) {
      *level = dbl_value + 1;
    }
    return true;
  }
  return false;
}

void container_config::set_config(TChannelConfig_Container *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  set_level(user_root, FIELD_WARNING_ABOVE_LEVEL, config->WarningAboveLevel);

  set_level(user_root, FIELD_ALARM_ABOVE_LEVEL, config->AlarmAboveLevel);

  set_level(user_root, FIELD_WARNING_BELOW_LEVEL, config->WarningBelowLevel);

  set_level(user_root, FIELD_ALARM_BELOW_LEVEL, config->AlarmBelowLevel);

  set_item_value(
      user_root,
      field_map.at(FIELD_MUTE_ALARM_SOUND_WITHOUT_ADDITIONAL_AUTH).c_str(),
      config->MuteAlarmSoundWithoutAdditionalAuth ? cJSON_True : cJSON_False,
      true, nullptr, nullptr, 0);

  cJSON *sensors = cJSON_CreateObject();

  for (size_t a = 0;
       a < sizeof(config->SensorInfo) / sizeof(TContainer_SensorInfo); a++) {
    if (config->SensorInfo[a].IsSet) {
      const char *channel_no =
          std::to_string(config->SensorInfo[a].ChannelNo).c_str();
      cJSON_DeleteItemFromObject(sensors, channel_no);

      cJSON *sensor = cJSON_CreateObject();

      set_item_value(sensor,
                     sensor_field_map.at(SENSOR_FIELD_FILL_LEVEL).c_str(),
                     cJSON_Number, true, nullptr, nullptr,
                     config->SensorInfo[a].FillLevel - 1);

      cJSON_AddItemToObject(sensors, channel_no, sensor);
    }
  }

  set_item_value(user_root, field_map.at(FIELD_SENSORS).c_str(), cJSON_Object,
                 true, sensors, nullptr, 0);
}

bool container_config::get_config(TChannelConfig_Container *config) {
  bool result = false;

  *config = {};

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  if (get_level(user_root, FIELD_WARNING_ABOVE_LEVEL,
                &config->WarningAboveLevel)) {
    result = true;
  }

  if (get_level(user_root, FIELD_ALARM_ABOVE_LEVEL, &config->AlarmAboveLevel)) {
    result = true;
  }

  if (get_level(user_root, FIELD_WARNING_BELOW_LEVEL,
                &config->WarningBelowLevel)) {
    result = true;
  }

  if (get_level(user_root, FIELD_ALARM_BELOW_LEVEL, &config->AlarmBelowLevel)) {
    result = true;
  }

  double dbl_value = 0;
  size_t n = 0;

  cJSON *sensors =
      cJSON_GetObjectItem(user_root, field_map.at(FIELD_SENSORS).c_str());
  if (sensors) {
    for (int a = 0; a < cJSON_GetArraySize(sensors); a++) {
      cJSON *item = cJSON_GetArrayItem(sensors, a);
      if (item && cJSON_IsObject(item)) {
        int channel_no = 0;
        try {
          channel_no = std::stoi(item->string);
        } catch (...) {
          channel_no = -1;
        }

        if (channel_no >= 0) {
          if (get_double(item,
                         sensor_field_map.at(SENSOR_FIELD_FILL_LEVEL).c_str(),
                         &dbl_value) &&
              dbl_value >= 0 && dbl_value <= 100) {
            config->SensorInfo[n].FillLevel = dbl_value + 1;
          }

          config->SensorInfo[n].ChannelNo = channel_no;
          config->SensorInfo[n].IsSet = 1;
          n++;

          if (n >= sizeof(config->SensorInfo) / sizeof(TContainer_SensorInfo)) {
            break;
          }
        }
      }
    }
  }

  return result;
}

void container_config::merge(supla_json_config *_dst) {
  container_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           true);
}
