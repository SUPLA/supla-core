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
#include "log.h"
#include "tools.h"

s_worker_action_rgb::s_worker_action_rgb(s_worker *worker)
    : s_worker_action(worker) {}

void s_worker_action_rgb::get_function_list(int list[FUNCTION_LIST_SIZE]) {
  list[0] = SUPLA_CHANNELFNC_DIMMER;
  list[1] = SUPLA_CHANNELFNC_RGBLIGHTING;
  list[2] = SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING;
}

int s_worker_action_rgb::try_limit(void) { return 2; }

int s_worker_action_rgb::waiting_time_to_retry(void) { return 30; }

int s_worker_action_rgb::waiting_time_to_check(void) { return 5; }

char s_worker_action_rgb::parse_rgbw_params(int *color, char *color_brightness,
                                            char *brightness, bool *random) {
  jsmn_parser p;
  jsmntok_t t[10];
  int a;
  int result = 0;
  int value = 0;

  if (color) *color = 0;

  if (color_brightness) *color_brightness = 0;

  if (brightness) *brightness = 0;

  if (worker->get_action_param() == NULL) {
    return 0;
  }

  jsmn_init(&p);
  int r = jsmn_parse(&p, worker->get_action_param(),
                     strnlen(worker->get_action_param(), 255), t,
                     sizeof(t) / sizeof(t[0]));

  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return 0;
  }

  for (a = 1; a < r - 1; a++) {
    if (jsoneq(worker->get_action_param(), &t[a], "hue") == 0) {
      if (jsoneq(worker->get_action_param(), &t[a + 1], "random") == 0) {
        if (color) {
          unsigned int seed = time(NULL);
          *color = st_hue2rgb(rand_r(&seed) % 360);

          if (random) {
            *random = true;
          }
        }

        result++;
      }
      if (jsoneq(worker->get_action_param(), &t[a + 1], "white") == 0) {
        if (color) {
          *color = 0xFFFFFF;
        }

        result++;

      } else if (json_get_int(&t[a + 1], &value)) {
        if (color) {
          *color = st_hue2rgb(value);
        }

        result++;
      }

    } else if (jsoneq(worker->get_action_param(), &t[a], "color_brightness") ==
               0) {
      if (json_get_int(&t[a + 1], &value) && value >= 0 && value <= 100) {
        if (color_brightness) {
          *color_brightness = value;
        }

        result++;
      }
    } else if (jsoneq(worker->get_action_param(), &t[a], "brightness") == 0) {
      if (json_get_int(&t[a + 1], &value) && value >= 0 && value <= 100) {
        if (brightness) {
          *brightness = value;
        }

        result++;
      }
    }
  }

  return result;
}

bool s_worker_action_rgb::check_result() {
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

void s_worker_action_rgb::do_action() {
  int color = 0;
  char color_brightness = 0;
  char brightness = 0;

  if (parse_rgbw_params(&color, &color_brightness, &brightness, NULL) > 0) {
    worker->ipcc_set_rgbw_value(color, color_brightness, brightness);
  }
}

REGISTER_ACTION(s_worker_action_rgb, ACTION_SET_RGBW_PARAMETERS);
