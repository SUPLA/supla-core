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

#include "action_set.h"

#include <string.h>

#include "json/cJSON.h"
#include "log.h"

s_worker_action_set::s_worker_action_set(s_abstract_worker *worker)
    : s_worker_action(worker) {}

bool s_worker_action_set::is_action_allowed(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL:
    case SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL:
      return true;
  }

  return false;
}

int s_worker_action_set::try_limit(void) { return 2; }

int s_worker_action_set::waiting_time_to_retry(void) { return 30; }

int s_worker_action_set::waiting_time_to_check(void) { return 5; }

bool s_worker_action_set::get_digiglass_params(int *active_bits, int *mask) {
  if (worker->get_action_param() == NULL || active_bits == NULL ||
      mask == NULL) {
    return false;
  }

  int result = 0;

  cJSON *root = cJSON_Parse(worker->get_action_param());
  if (root) {
    cJSON *item = cJSON_GetObjectItem(root, "mask");

    if (item && cJSON_IsNumber(item)) {
      *mask = item->valuedouble;
      result++;
    }

    item = cJSON_GetObjectItem(root, "activeBits");

    if (item && cJSON_IsNumber(item)) {
      *active_bits = item->valuedouble;
      result++;
    }

    cJSON_Delete(root);
  }

  return result == 2;
}

bool s_worker_action_set::result_success(int *fail_result_code) {
  int active_bits = 0;
  int mask = 0;
  if (get_digiglass_params(&active_bits, &mask)) {
    int current_mask = 0;
    return worker->ipcc_get_digiglass_value(&current_mask) &&
           (current_mask & active_bits) == mask;
  }

  return false;
}

bool s_worker_action_set::do_action() {
  int active_bits = 0;
  int mask = 0;
  if (get_digiglass_params(&active_bits, &mask)) {
    return worker->ipcc_set_digiglass_value(active_bits, mask);
  }

  return false;
}

REGISTER_ACTION(s_worker_action_set, ACTION_SET);
