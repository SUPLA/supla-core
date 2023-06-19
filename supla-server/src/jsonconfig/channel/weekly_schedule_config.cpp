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

#include "weekly_schedule_config.h"

#include "assert.h"
#include "log.h"

using std::string;

const char weekly_schedule_config::mode[] = "mode";
const char weekly_schedule_config::hvac[] = "hvac";
const char weekly_schedule_config::weekly_shedule[] = "weeklySchedule";
const char weekly_schedule_config::setpoint_temperature_min[] =
    "setpointTemperatureMin";
const char weekly_schedule_config::setpoint_temperature_max[] =
    "setpointTemperatureMax";
const char weekly_schedule_config::program_settings[] = "programSettings";
const char weekly_schedule_config::quarters[] = "quarters";

weekly_schedule_config::weekly_schedule_config(void) : channel_json_config() {}

weekly_schedule_config::weekly_schedule_config(supla_json_config *root)
    : channel_json_config(root) {}

void weekly_schedule_config::merge(supla_json_config *_dst) {
  if (!_dst) {
    return;
  }

  cJSON *src_ws_root = get_ws_root(get_user_root(), false);

  weekly_schedule_config dst(_dst);
  cJSON *dst_root = dst.get_user_root();

  if (!dst_root) {
    return;
  }

  cJSON *dst_hvac_root = cJSON_GetObjectItem(dst_root, hvac);
  if (!dst_hvac_root && src_ws_root) {
    dst_hvac_root = cJSON_AddObjectToObject(dst_root, hvac);
  }

  if (dst_hvac_root) {
    cJSON *dst_ws_root = cJSON_GetObjectItem(dst_hvac_root, weekly_shedule);
    if (dst_ws_root) {
      cJSON_DetachItemViaPointer(dst_hvac_root, dst_ws_root);
      cJSON_Delete(dst_ws_root);
    }

    if (src_ws_root) {
      dst_ws_root = cJSON_Duplicate(src_ws_root, cJSON_True);
      if (dst_ws_root) {
        cJSON_AddItemToObject(dst_hvac_root, weekly_shedule, dst_ws_root);
      }
    }
  }
}

cJSON *weekly_schedule_config::get_ws_root(cJSON *root, bool force) {
  if (root) {
    cJSON *hvac_root = cJSON_GetObjectItem(root, hvac);
    if (!hvac_root && force) {
      hvac_root = cJSON_AddObjectToObject(root, hvac);
    }
    if (hvac_root) {
      cJSON *ws_root = cJSON_GetObjectItem(hvac_root, weekly_shedule);
      if (!ws_root && force) {
        ws_root = cJSON_AddObjectToObject(hvac_root, weekly_shedule);
      }
      return ws_root;
    }
  }
  return nullptr;
}

cJSON *weekly_schedule_config::get_ws_root(void) {
  return get_ws_root(get_user_root(), true);
}

string weekly_schedule_config::mode_to_string(unsigned char mode) {
  switch (mode) {
    case SUPLA_HVAC_MODE_NOT_SET:
      return "NotSet";
    case SUPLA_HVAC_MODE_OFF:
      return "Off";
    case SUPLA_HVAC_MODE_HEAT:
      return "Heat";
    case SUPLA_HVAC_MODE_COOL:
      return "Cool";
    case SUPLA_HVAC_MODE_AUTO:
      return "Auto";
    case SUPLA_HVAC_MODE_FAN_ONLY:
      return "FanOnly";
    case SUPLA_HVAC_MODE_DRY:
      return "Dry";
  }

  return "";
}

unsigned char weekly_schedule_config::string_to_mode(const std::string &mode) {
  if (mode == "Off") {
    return SUPLA_HVAC_MODE_OFF;
  } else if (mode == "Heat") {
    return SUPLA_HVAC_MODE_HEAT;
  } else if (mode == "Cool") {
    return SUPLA_HVAC_MODE_COOL;
  } else if (mode == "Auto") {
    return SUPLA_HVAC_MODE_AUTO;
  } else if (mode == "FanOnly") {
    return SUPLA_HVAC_MODE_FAN_ONLY;
  } else if (mode == "Dry") {
    return SUPLA_HVAC_MODE_DRY;
  }

  return SUPLA_HVAC_MODE_NOT_SET;
}

void weekly_schedule_config::add_program(
    unsigned char index, TSD_ChannelConfig_WeeklySchedule *config,
    cJSON *program_root) {
  string name = std::to_string(index + 1);
  cJSON *program = cJSON_AddObjectToObject(program_root, name.c_str());
  if (program) {
    set_item_value(program, mode, cJSON_String, true,
                   mode_to_string(config->Program[index].Mode).c_str(), 0);

    set_item_value(program, setpoint_temperature_min, cJSON_Number, true,
                   nullptr, config->Program[index].Value1);

    set_item_value(program, setpoint_temperature_max, cJSON_Number, true,
                   nullptr, config->Program[index].Value2);
  }
}

void weekly_schedule_config::get_program(
    unsigned char index, TSD_ChannelConfig_WeeklySchedule *config,
    cJSON *program_root) {
  string name = std::to_string(index + 1);
  cJSON *program = cJSON_GetObjectItem(program_root, name.c_str());
  if (program) {
    string str_value;
    if (get_string(program, mode, &str_value)) {
      config->Program[index].Mode = string_to_mode(str_value);
    }

    double dbl_value = 0;
    if (get_double(program, setpoint_temperature_min, &dbl_value)) {
      config->Program[index].SetpointTemperatureMin = dbl_value;
    }
    if (get_double(program, setpoint_temperature_max, &dbl_value)) {
      config->Program[index].SetpointTemperatureMax = dbl_value;
    }
  }
}

void weekly_schedule_config::add_quarter(
    TSD_ChannelConfig_WeeklySchedule *config, cJSON *quarters_root,
    unsigned char quarter) {
  cJSON *item = cJSON_CreateNumber(quarter);
  assert(item != nullptr);
  cJSON_AddItemToArray(quarters_root, item);
}

void weekly_schedule_config::set_config(
    TSD_ChannelConfig_WeeklySchedule *config) {
  if (!config) {
    return;
  }

  cJSON *root = get_ws_root();
  if (!root) {
    return;
  }

  cJSON *program_root = cJSON_GetObjectItem(root, program_settings);
  if (program_root) {
    cJSON_DetachItemViaPointer(root, program_root);
    cJSON_Delete(program_root);
  }

  program_root = cJSON_AddObjectToObject(root, program_settings);

  if (program_root) {
    for (char a = 0; a < SUPLA_WEEKLY_SCHEDULE_PROGRAMS_MAX_SIZE; a++) {
      add_program(a, config, program_root);
    }
  }

  cJSON *quarters_root = cJSON_GetObjectItem(root, quarters);
  if (quarters_root) {
    cJSON_DetachItemViaPointer(root, quarters_root);
    cJSON_Delete(quarters_root);
  }

  quarters_root = cJSON_AddArrayToObject(root, quarters);

  for (unsigned short a = 0; a < sizeof(config->Quarters); a++) {
    unsigned char v = config->Quarters[a];
    add_quarter(config, quarters_root, v & 0xF);
    add_quarter(config, quarters_root, (v >> 4) & 0xF);
  }
}

void weekly_schedule_config::get_config(
    TSD_ChannelConfig_WeeklySchedule *config) {
  if (!config) {
    return;
  }

  *config = {};

  cJSON *root = get_ws_root();
  if (!root) {
    return;
  }

  cJSON *program_root = cJSON_GetObjectItem(root, program_settings);

  if (program_root) {
    for (char a = 0; a < SUPLA_WEEKLY_SCHEDULE_PROGRAMS_MAX_SIZE; a++) {
      get_program(a, config, program_root);
    }
  }

  cJSON *quarters_root = cJSON_GetObjectItem(root, quarters);
  if (quarters_root) {
    unsigned int arr_size = cJSON_GetArraySize(quarters_root);

    unsigned short offset = 0;

    for (unsigned short a = 0;
         a < arr_size && offset < sizeof(config->Quarters); a++) {
      cJSON *item = cJSON_GetArrayItem(quarters_root, a);

      if (item && cJSON_IsNumber(item)) {
        int v = cJSON_GetNumberValue(item);

        if (a % 2) {
          config->Quarters[offset] |= (v << 4) & 0xF0;
        } else {
          config->Quarters[offset] = v & 0xF;
        }
      }

      offset += a % 2;
    }
  }
}
