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

#include "action_valve_openclose.h"

s_worker_action_valve_openclose::s_worker_action_valve_openclose(
    s_worker *worker, bool doOpen)
    : s_worker_action(worker) {
  this->doOpen = doOpen;
}

s_worker_action_valve_open::s_worker_action_valve_open(s_worker *worker)
    : s_worker_action_valve_openclose(worker, true) {}

s_worker_action_valve_close::s_worker_action_valve_close(s_worker *worker)
    : s_worker_action_valve_openclose(worker, false) {}

void s_worker_action_valve_openclose::get_function_list(
    int list[FUNCTION_LIST_SIZE]) {
  list[0] = SUPLA_CHANNELFNC_VALVE_OPENCLOSE;
}

int s_worker_action_valve_openclose::try_limit(void) { return 3; }

int s_worker_action_valve_openclose::waiting_time_to_retry(void) { return 30; }

int s_worker_action_valve_openclose::waiting_time_to_check(void) { return 10; }

bool s_worker_action_valve_openclose::result_success(int *fail_result_code) {
  TValve_Value value;
  if (!worker->ipcc_get_valve_value(&value)) {
    return false;
  }

  if (!value.closed == doOpen) {
    return true;
  }

  if (fail_result_code && doOpen &&
      (value.flags & SUPLA_VALVE_FLAG_FLOODING ||
       value.flags & SUPLA_VALVE_FLAG_MANUALLY_CLOSED)) {
    *fail_result_code =
        ACTION_EXECUTION_RESULT_VALVE_CLOSED_MANUALLY_OR_FLOODING;
  }

  return false;
}

bool s_worker_action_valve_openclose::do_action() {
  if (doOpen) {
    TValve_Value value;
    if (!worker->ipcc_get_valve_value(&value) ||
        (value.flags & SUPLA_VALVE_FLAG_FLOODING ||
         value.flags & SUPLA_VALVE_FLAG_MANUALLY_CLOSED)) {
      return false;
    }
  }

  return worker->ipcc_set_char_value(doOpen ? 0 : 1);
}

REGISTER_ACTION(s_worker_action_valve_open, ACTION_OPEN);
REGISTER_ACTION(s_worker_action_valve_close, ACTION_CLOSE);
