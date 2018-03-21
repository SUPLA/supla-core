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

#include "action_shutreveal.h"
#include <string.h>
#include "log.h"

s_worker_action_shutreveal::s_worker_action_shutreveal(s_worker *worker,
                                                       RSActionKind kind)
    : s_worker_action(worker) {
  this->kind = kind;
}

s_worker_action_shut::s_worker_action_shut(s_worker *worker)
    : s_worker_action_shutreveal(worker, rsak_shut) {}

s_worker_action_reveal::s_worker_action_reveal(s_worker *worker)
    : s_worker_action_shutreveal(worker, rsak_reveal) {}

s_worker_action_reveal_partially::s_worker_action_reveal_partially(
    s_worker *worker)
    : s_worker_action_shutreveal(worker, rsak_reveal_partially) {}

void s_worker_action_shutreveal::get_function_list(
    int list[FUNCTION_LIST_SIZE]) {
  list[0] = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
}

int s_worker_action_shutreveal::try_limit(void) { return 2; }

int s_worker_action_shutreveal::waiting_time_to_retry(void) { return 120; }

int s_worker_action_shutreveal::waiting_time_to_check(void) { return 90; }

bool s_worker_action_shutreveal::check_result() {
  char value = 0;  // percent of shut
  char expected_value = 0;
  if (!worker->ipcc_get_char_value(&value)) {
    return false;
  }

  if (kind == rsak_shut) {
    expected_value = 100;
  } else if (kind == rsak_reveal) {
    expected_value = 0;
  } else if (kind == rsak_reveal_partially) {
    if (!parse_percentage(&expected_value)) {
      return false;
    }
    expected_value = 100 - expected_value;  // Convert shut to reveal
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

void s_worker_action_shutreveal::do_action() {
  char value = 0;
  char percent = 0;

  if (kind == rsak_shut) {
    value = 1;  // For compatibility purposes set to 1
  } else if (kind == rsak_reveal) {
    value = 2;  // For compatibility purposes set to 2
  } else if (kind == rsak_reveal_partially && parse_percentage(&percent)) {
    value = 110 - percent;  // 10 == 0%, 110 == 100%
  }

  worker->ipcc_set_char_value(value);
}

REGISTER_ACTION(s_worker_action_shut, ACTION_SHUT);
REGISTER_ACTION(s_worker_action_reveal, ACTION_REVEAL);
REGISTER_ACTION(s_worker_action_reveal_partially, ACTION_REVEAL_PARTIALLY);
