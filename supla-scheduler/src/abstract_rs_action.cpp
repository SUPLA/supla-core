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

#include "abstract_rs_action.h"

#include <stdlib.h>
#include <string.h>

#include "json/cJSON.h"
#include "log.h"

s_abstract_rs_action::s_abstract_rs_action(s_abstract_worker *worker)
    : s_worker_action(worker) {}

bool s_abstract_rs_action::is_action_allowed(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      return true;
  }
  return false;
}

int s_abstract_rs_action::try_limit(void) { return 2; }

int s_abstract_rs_action::waiting_time_to_retry(void) { return 120; }

int s_abstract_rs_action::waiting_time_to_check(void) { return 90; }

bool s_abstract_rs_action::parse_percentage(char *percent) {
  if (worker->get_action_param() == NULL || percent == NULL) {
    return false;
  }

  bool result = false;

  cJSON *root = cJSON_Parse(worker->get_action_param());
  if (root) {
    cJSON *item = cJSON_GetObjectItem(root, "percentage");

    if (item && cJSON_IsNumber(item) && item->valuedouble >= 0 &&
        item->valuedouble <= 100) {
      *percent = item->valuedouble;
      result = true;
    }
    cJSON_Delete(root);
  }

  return result;
}

bool s_abstract_rs_action::result_success(int *fail_result_code) {
  char value = 0;  // percent of shut
  char expected_value = 0;
  if (!worker->ipcc_get_char_value(&value)) {
    return false;
  }

  if (!get_expected_value(&expected_value)) {
    return false;
  }

  char sensor_value = worker->ipcc_get_opening_sensor_value();

  if (sensor_value != -1) {
    if (sensor_value != 1 && expected_value == 100) {
      value = 0;
    } else if (sensor_value == 1 && expected_value < 100) {
      value = 100;
    }
  }

  // supla_log(LOG_DEBUG, "Percent of shut: %i, expected value %i, diff: %i",
  //          value, expected_value, abs(expected_value - value));

  return abs(expected_value - value) <= 2;  // tolerance 2%
}
