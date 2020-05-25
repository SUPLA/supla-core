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

#include "action_openclose.h"
#include <string.h>
#include "log.h"

s_worker_action_openclose::s_worker_action_openclose(s_worker *worker,
                                                     bool doOpen)
    : s_worker_action(worker) {
  this->doOpen = doOpen;
}

void s_worker_action_openclose::get_function_list(
    int list[FUNCTION_LIST_SIZE]) {
  if (!worker->channel_group()) {
    list[0] = SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR;
    list[1] = SUPLA_CHANNELFNC_CONTROLLINGTHEGATE;
    list[2] = SUPLA_CHANNELFNC_VALVE_OPENCLOSE;
  }
}

bool s_worker_action_openclose::garage_func(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      return true;
  }

  return false;
}

bool s_worker_action_openclose::valve_func(void) {
  return worker->get_channel_func() == SUPLA_CHANNELFNC_VALVE_OPENCLOSE;
}

bool s_worker_action_openclose::check_before_start(void) {
  return garage_func();
}

s_worker_action_open::s_worker_action_open(s_worker *worker)
    : s_worker_action_openclose(worker, true) {}

void s_worker_action_open::get_function_list(int list[FUNCTION_LIST_SIZE]) {
  s_worker_action_openclose::get_function_list(list);
  list[3] = SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK;
  list[4] = SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK;
}

s_worker_action_close::s_worker_action_close(s_worker *worker)
    : s_worker_action_openclose(worker, false) {}

int s_worker_action_openclose::try_limit(void) {
  return garage_func() || valve_func() ? 4 : 1;
}

bool s_worker_action_openclose::retry_when_fail(void) {
  return valve_func() || garage_func() ? true
                                       : s_worker_action::retry_when_fail();
}

int s_worker_action_openclose::waiting_time_to_retry(void) {
  return garage_func() || valve_func() ? 60 : 30;
}

int s_worker_action_openclose::waiting_time_to_check(void) {
  return garage_func() || valve_func() ? 55 : 2;
}

bool s_worker_action_openclose::result_success(int *fail_result_code) {
  if (valve_func()) {
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
  } else if (garage_func()) {
    char sensor_value = worker->ipcc_get_opening_sensor_value();
    if (sensor_value == -1) {
      if (fail_result_code) {
        *fail_result_code = ACTION_EXECUTION_RESULT_NO_SENSOR;
      }
    } else {
      return doOpen == (sensor_value == 1 ? false : true);
    }

    return false;
  }

  return true;
}

bool s_worker_action_openclose::do_action() {
  if (valve_func()) {
    if (doOpen) {
      TValve_Value value;
      if (!worker->ipcc_get_valve_value(&value) ||
          (value.flags & SUPLA_VALVE_FLAG_FLOODING ||
           value.flags & SUPLA_VALVE_FLAG_MANUALLY_CLOSED)) {
        return false;
      }
    }

    return worker->ipcc_set_char_value(doOpen ? 1 : 0);
  } else if (!garage_func() || worker->ipcc_get_opening_sensor_value() != -1) {
    return worker->ipcc_set_char_value(1);
  }
  return false;
}

REGISTER_ACTION(s_worker_action_open, ACTION_OPEN);
REGISTER_ACTION(s_worker_action_close, ACTION_CLOSE);
