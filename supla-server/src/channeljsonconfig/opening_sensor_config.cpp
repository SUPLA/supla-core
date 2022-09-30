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

#include "opening_sensor_config.h"

// static
const char opening_sensor_config::retry_interrupt_key[] = "retryInterrupt";
// static
const char opening_sensor_config::related_channel_id_key[] =
    "openingSensorChannelId";
// static
const char opening_sensor_config::related_secondary_channel_id_key[] =
    "openingSensorSecondaryChannelId";
// static
const char opening_sensor_config::inverted_logic_key[] = "invertedLogic";

opening_sensor_config::opening_sensor_config(void) : channel_json_config() {}

opening_sensor_config::opening_sensor_config(channel_json_config *root)
    : channel_json_config(root) {}

int opening_sensor_config::get_int(const char *key) {
  cJSON *root = get_user_root();
  if (!root) {
    return 0;
  }

  cJSON *value = cJSON_GetObjectItem(root, key);
  if (value && cJSON_IsNumber(value)) {
    return value->valueint;
  }

  return 0;
}

bool opening_sensor_config::get_bool(const char *key) {
  cJSON *root = get_user_root();
  if (!root) {
    return false;
  }

  cJSON *value = cJSON_GetObjectItem(root, key);
  return value && cJSON_IsBool(value) && cJSON_IsTrue(value);
}

bool opening_sensor_config::get_retry_interrupt(void) {
  return get_bool(retry_interrupt_key);
}

int opening_sensor_config::get_related_channel_id(void) {
  return get_int(related_channel_id_key);
}

int opening_sensor_config::get_related_secondary_channel_id(void) {
  return get_int(related_secondary_channel_id_key);
}

bool opening_sensor_config::is_inverted_logic(void) {
  return get_bool(inverted_logic_key);
}
