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

#include "action_turn_onoff.h"

s_worker_action_turn_onoff::s_worker_action_turn_onoff(s_worker *worker,
                                                       bool setOn)
    : s_worker_action(worker) {
  this->setOn = setOn;
}

s_worker_action_turn_on::s_worker_action_turn_on(s_worker *worker)
    : s_worker_action_turn_onoff(worker, true) {}

s_worker_action_turn_off::s_worker_action_turn_off(s_worker *worker)
    : s_worker_action_turn_onoff(worker, false) {}

void s_worker_action_turn_onoff::get_function_list(
    int list[FUNCTION_LIST_SIZE]) {
  list[0] = SUPLA_CHANNELFNC_LIGHTSWITCH;
  list[1] = SUPLA_CHANNELFNC_POWERSWITCH;
}

int s_worker_action_turn_onoff::try_limit(void) { return 2; }

int s_worker_action_turn_onoff::waiting_time_to_retry(void) { return 30; }

int s_worker_action_turn_onoff::waiting_time_to_check(void) { return 5; }

bool s_worker_action_turn_onoff::check_result() {
  char value = 0;
  worker->ipcc_get_char_value(&value);

  return (value == 1) == setOn;
}

void s_worker_action_turn_onoff::do_action() {
  worker->ipcc_set_char_value(setOn ? 1 : 0);
}

REGISTER_ACTION(s_worker_action_turn_on, ACTION_TURN_ON);
REGISTER_ACTION(s_worker_action_turn_off, ACTION_TURN_OFF);
