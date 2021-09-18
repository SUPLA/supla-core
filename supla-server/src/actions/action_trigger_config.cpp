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

#include <actions/action_trigger_config.h>
#include <ctype.h>
#include <string.h>

#include "proto.h"

#define MAX_STR_LEN 20
#define ARR_NAME "actionTriggerCapabilities"
#define ACTIONS "actions"

_atc_map_t action_trigger_config::map[] = {
    {.cap = SUPLA_ACTION_CAP_TURN_ON, .str = "TURN_ON"},
    {.cap = SUPLA_ACTION_CAP_TURN_OFF, .str = "TURN_OFF"},
    {.cap = SUPLA_ACTION_CAP_TOGGLE_x1, .str = "TOGGLE_X1"},
    {.cap = SUPLA_ACTION_CAP_TOGGLE_x2, .str = "TOGGLE_X2"},
    {.cap = SUPLA_ACTION_CAP_TOGGLE_x3, .str = "TOGGLE_X3"},
    {.cap = SUPLA_ACTION_CAP_TOGGLE_x4, .str = "TOGGLE_X4"},
    {.cap = SUPLA_ACTION_CAP_TOGGLE_x5, .str = "TOGGLE_X5"},
    {.cap = SUPLA_ACTION_CAP_HOLD, .str = "HOLD"},
    {.cap = SUPLA_ACTION_CAP_SHORT_PRESS_x1, .str = "SHORT_PRESS_X1"},
    {.cap = SUPLA_ACTION_CAP_SHORT_PRESS_x2, .str = "SHORT_PRESS_X2"},
    {.cap = SUPLA_ACTION_CAP_SHORT_PRESS_x3, .str = "SHORT_PRESS_X3"},
    {.cap = SUPLA_ACTION_CAP_SHORT_PRESS_x4, .str = "SHORT_PRESS_X4"},
    {.cap = SUPLA_ACTION_CAP_SHORT_PRESS_x5, .str = "SHORT_PRESS_X5"}};

action_trigger_config::action_trigger_config(void) : channel_json_config() {}

action_trigger_config::action_trigger_config(channel_json_config *root)
    : channel_json_config(root) {}

bool action_trigger_config::equal(const char *str1, const char *str2) {
  if (!str1 || !str2) {
    return false;
  }

  size_t str_len = strnlen(str1, MAX_STR_LEN);

  if (str_len != strnlen(str2, MAX_STR_LEN)) {
    return false;
  }

  for (size_t a = 0; a < str_len; a++) {
    if (tolower(str1[a]) != tolower(str2[a])) {
      return false;
    }
  }

  return true;
}

const char *action_trigger_config::to_string(int cap) {
  int size = sizeof(map) / sizeof(_atc_map_t);

  for (int a = 0; a < size; a++) {
    if (map[a].cap == cap) {
      return map[a].str.c_str();
    }
  }

  return NULL;
}

int action_trigger_config::to_cap(const char *str) {
  int size = sizeof(map) / sizeof(_atc_map_t);

  for (int a = 0; a < size; a++) {
    if (equal(map[a].str.c_str(), str)) {
      return map[a].cap;
    }
  }

  return 0;
}

int action_trigger_config::to_cap(cJSON *item) {
  if (item) {
    return to_cap(cJSON_GetStringValue(item));
  }

  return 0;
}

unsigned int action_trigger_config::get_capabilities(void) {
  unsigned int result = 0;
  cJSON *json = get_properties_root();
  if (!json) {
    return result;
  }

  cJSON *st = cJSON_GetObjectItem(json, ARR_NAME);
  if (!st || !cJSON_IsArray(st)) {
    return result;
  }

  int st_size = cJSON_GetArraySize(st);

  for (short a = 0; a < st_size; a++) {
    result |= to_cap(cJSON_GetArrayItem(st, a));
  }

  return result;
}

bool action_trigger_config::set_capabilities(unsigned int caps) {
  cJSON *json = get_properties_root();
  if (!json) {
    return false;
  }

  int map_size = sizeof(map) / sizeof(_atc_map_t);

  unsigned int all_possible = 0;

  for (int a = 0; a < map_size; a++) {
    all_possible |= map[a].cap;
  }

  caps = all_possible & caps;

  if (caps == get_capabilities()) {
    return false;
  }

  cJSON *st = cJSON_GetObjectItem(json, ARR_NAME);
  if (st) {
    while (cJSON_GetArraySize(st)) {
      cJSON_DeleteItemFromArray(st, 0);
    }
  }

  if (!st) {
    st = cJSON_CreateArray();
    if (st) {
      cJSON_AddItemToObject(json, ARR_NAME, st);
    }
  }

  if (st) {
    for (int a = 0; a < map_size; a++) {
      if (caps & map[a].cap) {
        cJSON *jstr = cJSON_CreateString(map[a].str.c_str());
        if (jstr) {
          cJSON_AddItemToArray(st, jstr);
        }
      }
    }
  }

  return true;
}

unsigned int action_trigger_config::get_active_actions(void) {
  unsigned int result = 0;

  cJSON *json = get_user_root();
  if (!json) {
    return result;
  }

  unsigned int caps = get_capabilities();
  if (caps == 0) {
    return result;
  }

  cJSON *actions = cJSON_GetObjectItem(json, ACTIONS);

  int size = sizeof(map) / sizeof(_atc_map_t);

  for (int a = 0; a < size; a++) {
    if (cJSON_GetObjectItem(actions, map[a].str.c_str())) {
      result |= map[a].cap;
    }
  }

  return caps & result;
}
