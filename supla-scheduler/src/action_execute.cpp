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

#include "action_execute.h"

#include <string.h>

#include "json/cJSON.h"
#include "log.h"

s_worker_action_execute::s_worker_action_execute(s_abstract_worker *worker)
    : s_worker_action(worker) {}

bool s_worker_action_execute::is_action_allowed(void) {
  return worker->it_applies_to_scene();
}

int s_worker_action_execute::try_limit(void) { return 1; }

int s_worker_action_execute::waiting_time_to_retry(void) {
  return MIN_RETRY_TIME;
}

int s_worker_action_execute::waiting_time_to_check(void) {
  return MIN_CHECK_TIME;
}

bool s_worker_action_execute::result_success(int *fail_result_code) {
  return true;
}

bool s_worker_action_execute::do_action() {
  return worker->ipcc_execute_scene();
}

REGISTER_ACTION(s_worker_action_execute, ACTION_EXECUTE);
