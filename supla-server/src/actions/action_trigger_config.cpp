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

#include "actions/action_trigger_config.h"

#include <ctype.h>
#include <string.h>

#include "proto.h"
#include "tools.h"

#define MAX_STR_LEN 20

// static
const _atc_map_t action_trigger_config::map[] = {
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

// static
const char action_trigger_config::caps_key[] = "actionTriggerCapabilities";

// static
const char action_trigger_config::action_key[] = "action";

// static
const char action_trigger_config::actions_key[] = "actions";

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

bool action_trigger_config::equal(cJSON *item, const char *str) {
  return item && equal(cJSON_GetStringValue(item), str);
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

  cJSON *st = cJSON_GetObjectItem(json, caps_key);
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

  cJSON *st = cJSON_GetObjectItem(json, caps_key);
  if (st) {
    while (cJSON_GetArraySize(st)) {
      cJSON_DeleteItemFromArray(st, 0);
    }
  }

  if (!st) {
    st = cJSON_CreateArray();
    if (st) {
      cJSON_AddItemToObject(json, caps_key, st);
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

  cJSON *actions = cJSON_GetObjectItem(json, actions_key);

  int size = sizeof(map) / sizeof(_atc_map_t);

  for (int a = 0; a < size; a++) {
    if (cJSON_GetObjectItem(actions, map[a].str.c_str())) {
      result |= map[a].cap;
    }
  }

  return caps & result;
}

cJSON *action_trigger_config::get_cap_user_config(int cap) {
  if (!(get_capabilities() & cap)) {
    return NULL;
  }

  cJSON *root = get_user_root();
  if (!root) {
    return NULL;
  }

  cJSON *actions = cJSON_GetObjectItem(root, actions_key);

  if (!actions) {
    return NULL;
  }

  const char *cap_str = to_string(cap);

  if (!cap_str) {
    return NULL;
  }

  return cJSON_GetObjectItem(actions, cap_str);
}

_at_config_action_t action_trigger_config::get_action_assigned_to_capability(
    int cap) {
  _at_config_action_t result = {};

  if (!cap) {
    return result;
  }

  cJSON *cap_cfg = get_cap_user_config(cap);

  if (!cap_cfg) {
    return result;
  }

  cJSON *subjectId = cJSON_GetObjectItem(cap_cfg, "subjectId");
  if (subjectId && cJSON_IsNumber(subjectId)) {
    result.subjectId = subjectId->valueint;
  }

  cJSON *subjectType = cJSON_GetObjectItem(cap_cfg, "subjectType");
  result.channelGroup = equal(subjectType, "channelGroup");

  cJSON *action = cJSON_GetObjectItem(cap_cfg, action_key);

  if (action) {
    cJSON *id = cJSON_GetObjectItem(action, "id");
    if (id && cJSON_IsNumber(id)) {
      result.actionId = id->valueint;
    }
  }

  return result;
}

char action_trigger_config::get_percentage(int cap) {
  char result = -1;

  cJSON *cap_cfg = get_cap_user_config(cap);

  if (!cap_cfg) {
    return result;
  }

  cJSON *action = cJSON_GetObjectItem(cap_cfg, action_key);

  if (action) {
    cJSON *param = cJSON_GetObjectItem(action, "param");
    if (param) {
      cJSON *percentage = cJSON_GetObjectItem(param, "percentage");
      if (percentage && cJSON_IsNumber(percentage) &&
          percentage->valueint >= 0 && percentage->valueint <= 100) {
        result = percentage->valueint;
      }
    }
  }

  return result;
}

_at_config_rgbw_t action_trigger_config::get_rgbw(int cap) {
  _at_config_rgbw_t result = {.brightness = -1,
                              .color_brightness = -1,
                              .color = 0,
                              .color_random = false};
  cJSON *cap_cfg = get_cap_user_config(cap);

  if (!cap_cfg) {
    return result;
  }

  cJSON *action = cJSON_GetObjectItem(cap_cfg, action_key);

  if (action) {
    cJSON *param = cJSON_GetObjectItem(action, "param");
    if (param) {
      cJSON *item = cJSON_GetObjectItem(param, "brightness");
      if (item && cJSON_IsNumber(item) && item->valueint >= 0 &&
          item->valueint <= 100) {
        result.brightness = item->valueint;
      }

      item = cJSON_GetObjectItem(param, "color_brightness");
      if (item && cJSON_IsNumber(item) && item->valueint >= 0 &&
          item->valueint <= 100) {
        result.color_brightness = item->valueint;
      }

      item = cJSON_GetObjectItem(param, "hue");
      if (item) {
        if (cJSON_IsNumber(item)) {
          result.color = st_hue2rgb(item->valuedouble);
        } else if (equal(item, "white")) {
          result.color = 0xFFFFFF;
        } else if (equal(item, "random")) {
          while (!result.color) {
            struct timeval time;
            gettimeofday(&time, NULL);
            unsigned int seed = time.tv_sec + time.tv_usec;
            result.color = st_hue2rgb(rand_r(&seed) % 360);
          }

          result.color_random = true;
        }
      }
    }
  }

  return result;
}