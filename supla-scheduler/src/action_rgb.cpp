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

#include <ctime>

#include "log.h"
#include "proto.h"
#include "tools.h"

s_worker_action_rgb::s_worker_action_rgb(s_abstract_worker *worker)
    : s_worker_action(worker) {}

bool s_worker_action_rgb::is_action_allowed(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMER_CCT:
    case SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB:
      return true;
  }

  return false;
}

int s_worker_action_rgb::try_limit(void) { return 2; }

int s_worker_action_rgb::waiting_time_to_retry(void) { return 30; }

int s_worker_action_rgb::waiting_time_to_check(void) { return 5; }

bool s_worker_action_rgb::get_char_value(cJSON *root, const char *key_name,
                                         char *value) {
  cJSON *item = cJSON_GetObjectItem(root, key_name);
  if (item && cJSON_IsNumber(item)) {
    *value = item->valuedouble;
    return true;
  }
  return false;
}

char s_worker_action_rgb::parse_rgbw_params(int *color, char *color_brightness,
                                            char *brightness,
                                            bool *color_random, char *command,
                                            char *white_temperature) {
  int result = 0;

  if (color) *color = 0;

  if (color_brightness) *color_brightness = -1;

  if (brightness) *brightness = -1;

  if (command) *command = -1;

  if (white_temperature) *white_temperature = -1;

  if (worker->get_action_param() == NULL) {
    return 0;
  }

  cJSON *root = cJSON_Parse(worker->get_action_param());
  if (root) {
    cJSON *item = NULL;

    if (brightness && get_char_value(root, "brightness", brightness)) {
      result++;
    }

    if (color_brightness &&
        get_char_value(root, "color_brightness", color_brightness)) {
      result++;
    }

    if (white_temperature &&
        get_char_value(root, "white_temperature", white_temperature)) {
      result++;
    }

    if (command && get_char_value(root, "rgbw_command", command)) {
      result++;
    }

    if (color) {
      item = cJSON_GetObjectItem(root, "color");
      if (item) {
        char *value = cJSON_GetStringValue(item);
        if (value) {
          if (strnlen(value, 8) == 7 && value[0] == '#') {
            value++;
            char *end = nullptr;
            errno = 0;
            long lcolor = strtol(value, &end, 16);
            if (lcolor != 0 && end && *end == '\0') {
              *color = lcolor;
              if (color_random) *color_random = false;
              result++;
            }
          } else if (strncasecmp(value, "random", 10) == 0) {
            if (color_random) *color_random = true;
            result++;
          }
        }

      } else {
        item = cJSON_GetObjectItem(root, "hue");
        if (item) {
          if (cJSON_IsNumber(item)) {
            *color = st_hue2rgb(item->valuedouble);
          } else if (cJSON_IsString(item)) {
            if (strncasecmp(cJSON_GetStringValue(item), "random", 255) == 0) {
              if (color_random) {
                *color_random = true;
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
    }

    cJSON_Delete(root);
  }

  return result;
}

bool s_worker_action_rgb::result_success(int *fail_result_code) {
  int color, expected_color = 0;
  char color_brightness, expected_color_brightness = 0;
  char brightness, expected_brightness = 0;
  char white_temperature, expected_white_temperature = 0;
  char command = 0;
  bool color_random = false;

  if (parse_rgbw_params(&expected_color, &expected_color_brightness,
                        &expected_brightness, &color_random, &command,
                        &expected_white_temperature) == 0) {
    return false;
  }

  if (!worker->ipcc_get_rgbw_value(&color, &color_brightness, &brightness,
                                   &white_temperature)) {
    return false;
  }

  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB:
      return (color_random || color == expected_color || !expected_color) &&
             (color_brightness == expected_color_brightness ||
              expected_color_brightness == -1) &&
             (white_temperature == expected_white_temperature ||
              expected_white_temperature == -1) &&
             (brightness == expected_brightness || expected_brightness == -1);
    case SUPLA_CHANNELFNC_DIMMER_CCT:
      return (white_temperature == expected_white_temperature ||
              expected_white_temperature == -1) ||
             (brightness == expected_brightness || expected_brightness == -1);
    case SUPLA_CHANNELFNC_DIMMER:
      return brightness == expected_brightness || expected_brightness == -1;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      return (color_random || color == expected_color || !expected_color) &&
             (color_brightness == expected_color_brightness ||
              expected_color_brightness == -1);
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return (color_random || color == expected_color || !expected_color) &&
             (color_brightness == expected_color_brightness ||
              expected_color_brightness == -1) &&
             (brightness == expected_brightness || expected_brightness == -1);
      break;
  }

  return false;
}

bool s_worker_action_rgb::do_action() {
  int color = 0;
  char color_brightness = -1;
  char brightness = -1;
  char command = 0;
  char white_temperature = -1;
  bool random;

  if (parse_rgbw_params(&color, &color_brightness, &brightness, &random,
                        &command, &white_temperature) > 0) {
    return worker->ipcc_set_rgbw_value(color, color_brightness, brightness,
                                       random, command, white_temperature);
  }
  return false;
}

REGISTER_ACTION(s_worker_action_rgb, ACTION_SET_RGBW_PARAMETERS);
