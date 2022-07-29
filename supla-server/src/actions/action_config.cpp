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

#include "action_config.h"

#include <stdlib.h>
#include <strings.h>
#include <time.h>

#include "json/cJSON.h"
#include "tools.h"

supla_action_config::supla_action_config(void) : abstract_action_config() {
  action_id = 0;
  subject_type = stUnknown;
  subject_id = 0;
  source_device_id = 0;
  source_channel_id = 0;
  percentage = 0;
  rgbw = {};
}

supla_action_config::~supla_action_config(void) {}

int supla_action_config::get_action_id(void) { return action_id; }

void supla_action_config::set_action_id(int action_id) {
  this->action_id = action_id;
}

_subjectType_e supla_action_config::get_subject_type(void) {
  return subject_type;
}

void supla_action_config::set_subject_type(_subjectType_e subject_type) {
  this->subject_type = subject_type;
}

int supla_action_config::get_subject_id(void) { return subject_id; }

void supla_action_config::set_subject_id(int subject_id) {
  this->subject_id = subject_id;
}

int supla_action_config::get_source_device_id(void) { return source_device_id; }

void supla_action_config::set_source_device_id(int source_device_id) {
  this->source_device_id = source_device_id;
}

int supla_action_config::get_source_channel_id(void) {
  return source_channel_id;
}

void supla_action_config::set_source_channel_id(int source_channel_id) {
  this->source_channel_id = source_channel_id;
}

char supla_action_config::get_percentage(void) { return percentage; }

void supla_action_config::set_percentage(char percentage) {
  this->percentage = percentage;
}

_action_config_rgbw_t supla_action_config::get_rgbw(void) {
  if (rgbw.color_random) {
    struct timeval now = {};
    gettimeofday(&now, NULL);
    unsigned int seed = now.tv_sec + now.tv_usec;
    rgbw.color = st_hue2rgb(rand_r(&seed) % 360);
  }
  return rgbw;
}

void supla_action_config::set_rgbw(_action_config_rgbw_t rgbw) {
  this->rgbw = rgbw;
}

void supla_action_config::apply_json_params(const char *params) {
  cJSON *root = cJSON_Parse(params);
  if (!root) {
    return;
  }

  cJSON *item = cJSON_GetObjectItem(root, "percentage");

  if (item) {
    if (cJSON_IsNumber(item)) {
      if (item->valuedouble < 0) {
        set_percentage(0);
      } else if (item->valuedouble > 100) {
        set_percentage(100);
      } else {
        set_percentage(item->valuedouble);
      }
    }
  }

  item = cJSON_GetObjectItem(root, "sourceChannelId");

  if (item && cJSON_IsNumber(item)) {
    set_source_channel_id(item->valuedouble);
  }

  item = cJSON_GetObjectItem(root, "sourceDeviceId");

  if (item && cJSON_IsNumber(item)) {
    set_source_device_id(item->valuedouble);
  }

  _action_config_rgbw_t rgbw = {};
  bool rgbw_changed = false;

  item = cJSON_GetObjectItem(root, "brightness");
  if (item && cJSON_IsNumber(item)) {
    if (item->valuedouble < 0) {
      rgbw.brightness = 0;
    } else if (item->valuedouble > 100) {
      rgbw.brightness = 100;
    } else {
      rgbw.brightness = item->valuedouble;
    }

    rgbw_changed = true;
  }

  item = cJSON_GetObjectItem(root, "color_brightness");
  if (item && cJSON_IsNumber(item)) {
    if (item->valuedouble < 0) {
      rgbw.color_brightness = 0;
    } else if (item->valuedouble > 100) {
      rgbw.color_brightness = 100;
    } else {
      rgbw.color_brightness = item->valuedouble;
    }
    rgbw_changed = true;
  }

  item = cJSON_GetObjectItem(root, "hue");
  if (item) {
    if (cJSON_IsNumber(item)) {
      rgbw.color = st_hue2rgb(item->valuedouble);
      rgbw_changed = true;
    } else if (cJSON_IsString(item)) {
      if (strncasecmp(cJSON_GetStringValue(item), "random", 255) == 0) {
        rgbw.color_random = true;
        rgbw_changed = true;
      } else if (strncasecmp(cJSON_GetStringValue(item), "white", 255) == 0) {
        rgbw.color = 0xFFFFFF;
        rgbw_changed = true;
      }
    }
  }

  if (rgbw_changed) {
    set_rgbw(rgbw);
  }

  cJSON_Delete(root);
}
