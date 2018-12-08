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
  noSensor = false;
}

void s_worker_action_openclose::get_function_list(
    int list[FUNCTION_LIST_SIZE]) {
  if (!worker->channel_group()) {
    list[0] = SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR;
    list[1] = SUPLA_CHANNELFNC_CONTROLLINGTHEGATE;
  }
}

bool s_worker_action_openclose::no_sensor(void) { return noSensor; }

bool s_worker_action_openclose::garage_group(void) {
  switch (worker->get_channel_func()) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      return true;
  }

  return false;
}

bool s_worker_action_openclose::check_before_start(void) {
  return garage_group();
}

s_worker_action_open::s_worker_action_open(s_worker *worker)
    : s_worker_action_openclose(worker, true) {}

void s_worker_action_open::get_function_list(int list[FUNCTION_LIST_SIZE]) {
  s_worker_action_openclose::get_function_list(list);
  list[2] = SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK;
  list[3] = SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK;
}

s_worker_action_close::s_worker_action_close(s_worker *worker)
    : s_worker_action_openclose(worker, false) {}

int s_worker_action_openclose::try_limit(void) {
  return garage_group() ? 4 : 1;
}

bool s_worker_action_openclose::retry_when_fail(void) {
  return garage_group() ? true : s_worker_action::retry_when_fail();
}

int s_worker_action_openclose::waiting_time_to_retry(void) {
  return garage_group() ? 60 : 30;
}

int s_worker_action_openclose::waiting_time_to_check(void) {
  return garage_group() ? 55 : 2;
}

bool s_worker_action_openclose::check_result() {
  noSensor = false;

  if (garage_group()) {
    char sensor_value = worker->ipcc_get_opening_sensor_value();
    if (sensor_value == -1) {
      noSensor = true;
    } else {
      return doOpen == (sensor_value == 1 ? false : true);
    }

    return false;
  }

  return true;
}

void s_worker_action_openclose::do_action() {
  if (!garage_group() || worker->ipcc_get_opening_sensor_value() != -1) {
    worker->ipcc_set_char_value(1);
  }
}

REGISTER_ACTION(s_worker_action_open, ACTION_OPEN);
REGISTER_ACTION(s_worker_action_close, ACTION_CLOSE);
