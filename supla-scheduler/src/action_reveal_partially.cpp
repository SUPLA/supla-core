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

#include "action_reveal_partially.h"

#include <stdlib.h>
#include <string.h>

#include "json/cJSON.h"
#include "log.h"

s_worker_action_reveal_partially::s_worker_action_reveal_partially(
    s_abstract_worker *worker)
    : s_abstract_rs_action(worker) {}

bool s_worker_action_reveal_partially::get_expected_value(
    char *expected_value) {
  if (!parse_percentage(expected_value)) {
    return false;
  }

  *expected_value = 100 - *expected_value;
  return true;
}

bool s_worker_action_reveal_partially::do_action() {
  char percent = 0;

  if (parse_percentage(&percent)) {
    percent = 110 - percent;  // 10 == 0%, 110 == 100%

    return worker->ipcc_set_char_value(percent);
  }

  return false;
}

REGISTER_ACTION(s_worker_action_reveal_partially, ACTION_REVEAL_PARTIALLY);
