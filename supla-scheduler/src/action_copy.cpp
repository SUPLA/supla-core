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

#include "action_copy.h"

#include <string.h>

#include "json/cJSON.h"
#include "log.h"

s_worker_action_copy::s_worker_action_copy(s_abstract_worker *worker)
    : s_worker_action(worker) {}

void s_worker_action_copy::get_function_list(int list[FUNCTION_LIST_SIZE]) {
  list[0] = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
  list[1] = SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW;
  list[2] = SUPLA_CHANNELFNC_CONTROLLINGTHEGATE;
  list[3] = SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR;
  list[4] = SUPLA_CHANNELFNC_LIGHTSWITCH;
  list[5] = SUPLA_CHANNELFNC_POWERSWITCH;
  list[6] = SUPLA_CHANNELFNC_STAIRCASETIMER;
  list[7] = SUPLA_CHANNELFNC_DIMMER;
  list[8] = SUPLA_CHANNELFNC_RGBLIGHTING;
  list[9] = SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING;
}

int s_worker_action_copy::try_limit(void) { return 1; }

int s_worker_action_copy::waiting_time_to_retry(void) { return MIN_RETRY_TIME; }

int s_worker_action_copy::waiting_time_to_check(void) { return MIN_CHECK_TIME; }

bool s_worker_action_copy::get_params(int *sourceDeviceId,
                                      int *sourceChannelId) {
  if (worker->get_action_param() == NULL || sourceChannelId == NULL) {
    return false;
  }

  if (sourceDeviceId) {
    *sourceDeviceId = 0;
  }

  *sourceChannelId = 0;

  cJSON *root = cJSON_Parse(worker->get_action_param());
  if (root) {
    cJSON *item = cJSON_GetObjectItem(root, "sourceChannelId");

    if (item && cJSON_IsNumber(item)) {
      *sourceChannelId = item->valuedouble;
    }

    if (sourceDeviceId) {
      item = cJSON_GetObjectItem(root, "sourceDeviceId");

      if (item && cJSON_IsNumber(item)) {
        *sourceDeviceId = item->valuedouble;
      }
    }

    cJSON_Delete(root);
  }

  return *sourceChannelId;
}

bool s_worker_action_copy::result_success(int *fail_result_code) {
  return true;
}

bool s_worker_action_copy::do_action() {
  int sourceDeviceId = 0;
  int sourceChannelId = 0;
  if (get_params(&sourceDeviceId, &sourceChannelId)) {
    return worker->ipcc_action_copy(sourceDeviceId, sourceChannelId);
  }

  return false;
}

REGISTER_ACTION(s_worker_action_copy, ACTION_COPY);
