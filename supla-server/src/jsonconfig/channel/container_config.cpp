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

#include <cstdint>
#include <map>
#include <string>

#include "log.h"

using std::map;
using std::string;

#define FIELD_WARNING_ABOVE_LEVEL 1
#define FIELD_ALARM_ABOVE_LEVEL 2
#define FIELD_WARNING_BELOW_LEVEL 3
#define FIELD_ALARM_BELOW_LEVEL 4
#define FIELD_MUTE_ALARM_SOUND_WITHOUT_ADDITIONAL_AUTH 5
#define FIELD_SENSORS 6

#define SENSOR_FIELD_CHANNEL_ID 1
#define SENSOR_FIELD_FILL_LEVEL 2

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
        {SENSOR_FIELD_CHANNEL_ID, "channelId"},
        {SENSOR_FIELD_FILL_LEVEL, "fillLevel"},
};

container_config::container_config(supla_json_config *root)
    : supla_json_config(root) {}

container_config::container_config(void) : supla_json_config() {}

void container_config::set_config(TChannelConfig_Container *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  set_level(field_map, user_root, FIELD_WARNING_ABOVE_LEVEL,
            config->WarningAboveLevel, 100);

  set_level(field_map, user_root, FIELD_ALARM_ABOVE_LEVEL,
            config->AlarmAboveLevel, 100);

  set_level(field_map, user_root, FIELD_WARNING_BELOW_LEVEL,
            config->WarningBelowLevel, 100);

  set_level(field_map, user_root, FIELD_ALARM_BELOW_LEVEL,
            config->AlarmBelowLevel, 100);

  set_item_value(
      user_root,
      field_map.at(FIELD_MUTE_ALARM_SOUND_WITHOUT_ADDITIONAL_AUTH).c_str(),
      config->MuteAlarmSoundWithoutAdditionalAuth ? cJSON_True : cJSON_False,
      true, nullptr, nullptr, 0);

  cJSON *sensors = cJSON_CreateArray();

  // Canonical fixed slots: never compact or deduplicate reference positions.
  for (const auto &entry : config->SensorInfo) {
    cJSON *sensor = cJSON_CreateObject();
    cJSON_AddItemToObject(sensor, "channelId",
                          entry.ChannelId > 0
                              ? cJSON_CreateNumber(entry.ChannelId)
                              : cJSON_CreateNull());
    cJSON_AddItemToObject(sensor, "fillLevel",
                          entry.FillLevel <= 100
                              ? cJSON_CreateNumber(entry.FillLevel)
                              : cJSON_CreateNull());
    cJSON_AddItemToArray(sensors, sensor);
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

  int level = 0;

  if (get_level(field_map, user_root, FIELD_WARNING_ABOVE_LEVEL, &level, 100)) {
    config->WarningAboveLevel = level;
    result = true;
  }

  if (get_level(field_map, user_root, FIELD_ALARM_ABOVE_LEVEL, &level, 100)) {
    config->AlarmAboveLevel = level;
    result = true;
  }

  if (get_level(field_map, user_root, FIELD_WARNING_BELOW_LEVEL, &level, 100)) {
    config->WarningBelowLevel = level;
    result = true;
  }

  if (get_level(field_map, user_root, FIELD_ALARM_BELOW_LEVEL, &level, 100)) {
    config->AlarmBelowLevel = level;
    result = true;
  }

  bool bool_value = false;

  if (get_bool(
          user_root,
          field_map.at(FIELD_MUTE_ALARM_SOUND_WITHOUT_ADDITIONAL_AUTH).c_str(),
          &bool_value)) {
    config->MuteAlarmSoundWithoutAdditionalAuth = bool_value ? 1 : 0;
    result = true;
  }

  cJSON *sensors = cJSON_GetObjectItem(user_root, "sensors");
  if (cJSON_IsArray(sensors)) {
    result = true;
    for (int slot = 0; slot < 10; ++slot) {
      cJSON *item = cJSON_GetArrayItem(sensors, slot);
      double id = 0, level = 0;
      if (get_double(item, "channelId", &id) && id > 0 && id <= INT32_MAX &&
          id == static_cast<int32_t>(id)) {
        config->SensorInfo[slot].ChannelId = static_cast<int32_t>(id);
      }
      if (get_double(item, "fillLevel", &level) && level >= 0 && level <= 100 &&
          level == static_cast<int>(level)) {
        config->SensorInfo[slot].FillLevel = level;
      }
    }
  }

  return result;
}

void container_config::merge(supla_json_config *_dst) {
  container_config dst(_dst);
  container_config input;
  input = *this;
  if (device_local_reference) {
    TChannelConfig_Container incoming = {}, current = {};
    input.get_config(&incoming);
    dst.get_config(&current);
    for (size_t slot = 0; slot < 10; ++slot) {
      unsigned int old_id = current.SensorInfo[slot].ChannelId;
      unsigned int new_id = incoming.SensorInfo[slot].ChannelId;
      if ((old_id && !device_local_reference(old_id)) ||
          !device_local_reference(new_id)) {
        incoming.SensorInfo[slot].ChannelId = old_id;
      }
    }
    input.set_config(&incoming);
  }
  supla_json_config::merge(input.get_user_root(), dst.get_user_root(),
                           field_map, true);
}
