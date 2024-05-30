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

#include "abstract_action_shut_partially.h"

#include <stdlib.h>
#include <string.h>

#include "json/cJSON.h"
#include "log.h"

s_abstract_action_shut_partially::s_abstract_action_shut_partially(
    s_abstract_worker *worker)
    : s_worker_action(worker) {}

s_abstract_action_shut_partially::~s_abstract_action_shut_partially(void) {}

bool s_abstract_action_shut_partially::is_action_allowed(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_TERRACE_AWNING:
    case SUPLA_CHANNELFNC_PROJECTOR_SCREEN:
    case SUPLA_CHANNELFNC_CURTAIN:
    case SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
    case SUPLA_CHANNELFNC_VERTICAL_BLIND:
      return true;
  }
  return false;
}

int s_abstract_action_shut_partially::try_limit(void) { return 2; }

int s_abstract_action_shut_partially::waiting_time_to_retry(void) {
  return 120;
}

int s_abstract_action_shut_partially::waiting_time_to_check(void) { return 90; }

bool s_abstract_action_shut_partially::get_expected(char *percentage,
                                                    bool *percentage_as_delta,
                                                    char *tilt,
                                                    bool *tilt_as_delta) {
  if (worker->get_action_param() == NULL || percentage == nullptr ||
      percentage_as_delta == nullptr || tilt == nullptr ||
      tilt_as_delta == nullptr) {
    return false;
  }

  *percentage = -1;
  *percentage_as_delta = false;
  *tilt = -1;
  *tilt_as_delta = false;

  bool result = false;

  cJSON *root = cJSON_Parse(worker->get_action_param());
  if (root) {
    cJSON *item = cJSON_GetObjectItem(root, "percentage");

    if (item && cJSON_IsNumber(item) && item->valuedouble >= 0 &&
        item->valuedouble <= 100) {
      *percentage = item->valuedouble;
      result = true;
    } else {
      item = cJSON_GetObjectItem(root, "percentageDelta");

      if (item && cJSON_IsNumber(item) && item->valuedouble >= -100 &&
          item->valuedouble <= 100) {
        *percentage = item->valuedouble;
        *percentage_as_delta = true;
        result = true;
      }
    }

    item = cJSON_GetObjectItem(root, "tilt");

    if (item && cJSON_IsNumber(item) && item->valuedouble >= 0 &&
        item->valuedouble <= 100) {
      *tilt = item->valuedouble;
      result = true;
    } else {
      item = cJSON_GetObjectItem(root, "tiltDelta");

      if (item && cJSON_IsNumber(item) && item->valuedouble >= -100 &&
          item->valuedouble <= 100) {
        *tilt = item->valuedouble;
        *tilt_as_delta = true;
        result = true;
      }
    }
    cJSON_Delete(root);
  }

  return result;
}

bool s_abstract_action_shut_partially::result_success(int *fail_result_code) {
  char expected_percentage = -1;
  bool expected_percentage_as_delta = false;
  char expected_tilt = -1;
  bool expected_tilt_as_delta = false;

  if (!get_expected(&expected_percentage, &expected_percentage_as_delta,
                    &expected_tilt, &expected_tilt_as_delta)) {
    return false;
  }

  char percentage = -1;
  char tilt = -1;

  if (worker->get_channel_func() ==
          SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND ||
      worker->get_channel_func() == SUPLA_CHANNELFNC_VERTICAL_BLIND) {
    if (!worker->ipcc_get_fb_value(&percentage, &tilt)) {
      return false;
    }
  } else {
    expected_tilt = -1;
    expected_tilt_as_delta = false;

    if (!worker->ipcc_get_char_value(&percentage)) {
      return false;
    }
  }

  if (!expected_percentage_as_delta &&
      (worker->get_channel_func() ==
           SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER ||
       worker->get_channel_func() ==
           SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW)) {
    char sensor_value = worker->ipcc_get_opening_sensor_value();

    if (sensor_value != -1) {
      if (sensor_value != 1 && expected_percentage == 100) {
        percentage = 0;
      } else if (sensor_value == 1 && expected_percentage < 100) {
        percentage = 100;
      }
    }
  }

  bool tilt_match = true;
  bool percentage_match = true;

  if (expected_tilt_as_delta == false && expected_tilt != -1 &&
      abs(expected_tilt - tilt) > 2) {  // tolerance two percentage points
    tilt_match = false;
  }

  if (expected_percentage_as_delta == false && expected_percentage != -1 &&
      abs(expected_percentage - percentage) >
          2) {  // tolerance two percentage points
    percentage_match = false;
  }

  return tilt_match || percentage_match;
}

bool s_abstract_action_shut_partially::do_action() {
  char percentage = -1;
  bool percentage_as_delta = false;
  char tilt = -1;
  bool tilt_as_delta = false;

  if (!get_expected(&percentage, &percentage_as_delta, &tilt, &tilt_as_delta)) {
    return false;
  }

  return worker->ipcc_action_shut_partially(percentage, percentage_as_delta,
                                            tilt, tilt_as_delta);
}
