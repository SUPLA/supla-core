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

#include "action_rgb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json/cJSON.h"
#include "log.h"
#include "tools.h"

s_worker_action_rgb::s_worker_action_rgb(s_abstract_worker *worker)
    : s_worker_action(worker) {}

bool s_worker_action_rgb::is_action_allowed(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return true;
  }

  return false;
}

int s_worker_action_rgb::try_limit(void) { return 2; }

int s_worker_action_rgb::waiting_time_to_retry(void) { return 30; }

int s_worker_action_rgb::waiting_time_to_check(void) { return 5; }

char s_worker_action_rgb::parse_rgbw_params(int *color, char *color_brightness,
                                            char *brightness, bool *random) {
  int result = 0;

  if (color) *color = 0;

  if (color_brightness) *color_brightness = 0;

  if (brightness) *brightness = 0;

  if (worker->get_action_param() == NULL) {
    return 0;
  }

  cJSON *root = cJSON_Parse(worker->get_action_param());
  if (root) {
    cJSON *item = NULL;

    if (brightness) {
      item = cJSON_GetObjectItem(root, "brightness");
      if (item && cJSON_IsNumber(item) && item->valuedouble >= 0 &&
          item->valuedouble <= 100) {
        *brightness = item->valuedouble;
        result++;
      }
    }

    if (color_brightness) {
      item = cJSON_GetObjectItem(root, "color_brightness");
      if (item && cJSON_IsNumber(item) && item->valuedouble >= 0 &&
          item->valuedouble <= 100) {
        *color_brightness = item->valuedouble;
        result++;
      }
    }
    if (color) {
      item = cJSON_GetObjectItem(root, "hue");
      if (item) {
        if (cJSON_IsNumber(item)) {
          *color = st_hue2rgb(item->valuedouble);
        } else if (cJSON_IsString(item)) {
          if (strncasecmp(cJSON_GetStringValue(item), "random", 255) == 0) {
            unsigned int seed = time(NULL);
            *color = st_hue2rgb(rand_r(&seed) % 360);
            if (random) {
              *random = true;
            }
            result++;
          } else if (strncasecmp(cJSON_GetStringValue(item), "white", 255) ==
                     0) {
            *color = 0xFFFFFF;
            result++;
          }
        }
      }
    }

    cJSON_Delete(root);
  }

  return result;
}

bool s_worker_action_rgb::result_success(int *fail_result_code) {
  int color, expected_color = 0;
  char color_brightness, expected_color_brightness = 0;
  char brightness, expected_brightness = 0;
  bool random = false;

  if (parse_rgbw_params(&expected_color, &expected_color_brightness,
                        &expected_brightness, &random) == 0) {
    return false;
  }

  if (!worker->ipcc_get_rgbw_value(&color, &color_brightness, &brightness)) {
    return false;
  }

  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_DIMMER:
      return brightness == expected_brightness;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      return (random || color == expected_color) &&
             color_brightness == expected_color_brightness;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return (random || color == expected_color) &&
             color_brightness == expected_color_brightness &&
             brightness == expected_brightness;
      break;
  }

  return false;
}

bool s_worker_action_rgb::do_action() {
  int color = 0;
  char color_brightness = 0;
  char brightness = 0;

  if (parse_rgbw_params(&color, &color_brightness, &brightness, NULL) > 0) {
    return worker->ipcc_set_rgbw_value(color, color_brightness, brightness);
  }
  return false;
}

REGISTER_ACTION(s_worker_action_rgb, ACTION_SET_RGBW_PARAMETERS);
