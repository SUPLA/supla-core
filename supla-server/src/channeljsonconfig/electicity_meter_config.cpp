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

#include "electricity_meter_config.h"

const _emc_map_t electricity_meter_config::map[] = {
    {.var = EM_VAR_FORWARD_ACTIVE_ENERGY, .str = "forwardActiveEnergy"},
    {.var = EM_VAR_REVERSE_ACTIVE_ENERGY, .str = "reverseActiveEnergy"},
    {.var = EM_VAR_FORWARD_REACTIVE_ENERGY, .str = "forwardReactiveEnergy"},
    {.var = EM_VAR_REVERSE_REACTIVE_ENERGY, .str = "reverseReactiveEnergy"},
    {.var = EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED,
     .str = "forwardActiveEnergyBalanced"},
    {.var = EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED,
     .str = "reverseActiveEnergyBalanced"}};

// static
const char electricity_meter_config::counters_available_key[] =
    "countersAvailable";

electricity_meter_config::electricity_meter_config(void)
    : channel_json_config() {}

electricity_meter_config::electricity_meter_config(channel_json_config *root)
    : channel_json_config(root) {}

int electricity_meter_config::get_map_size(void) {
  return sizeof(map) / sizeof(_emc_map_t);
}

int electricity_meter_config::get_map_key(int index) { return map[index].var; }

const char *electricity_meter_config::get_map_str(int index) {
  return map[index].str.c_str();
}

int electricity_meter_config::get_available_counters(void) {
  cJSON *root = get_properties_root();
  if (!root) {
    return 0;
  }

  cJSON *available = cJSON_GetObjectItem(root, counters_available_key);
  if (!available || !cJSON_IsArray(available)) {
    return 0;
  }

  int result = 0;
  int asize = cJSON_GetArraySize(available);

  for (int a = 0; a < asize; a++) {
    cJSON *item = cJSON_GetArrayItem(available, a);
    if (item && cJSON_IsString(item)) {
      result |= key_with_string(cJSON_GetStringValue(item));
    }
  }

  return result;
}

bool electricity_meter_config::update_available_counters(int measured_values) {
  cJSON *root = get_properties_root();
  if (!root) {
    return false;
  }

  cJSON *available = cJSON_GetObjectItem(root, counters_available_key);

  if (available && !cJSON_IsArray(available)) {
    return false;
  }

  int available_counters = get_available_counters();

  bool result = false;
  int msize = get_map_size();

  for (int a = 0; a < msize; a++) {
    if ((measured_values & get_map_key(a)) &&
        (available_counters & get_map_key(a)) == 0) {
      if (!available) {
        available = cJSON_AddArrayToObject(root, counters_available_key);

        if (!available) {
          return false;
        }
      }

      cJSON *jstr = cJSON_CreateString(get_map_str(a));
      if (jstr) {
        cJSON_AddItemToArray(available, jstr);
        result = true;
      }
    }
  }

  return result;
}
