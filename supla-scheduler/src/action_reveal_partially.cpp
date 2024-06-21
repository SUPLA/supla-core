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
    : s_abstract_action_shut_partially(worker) {}

s_worker_action_reveal_partially::~s_worker_action_reveal_partially(void) {}

bool s_worker_action_reveal_partially::get_expected(char *percentage,
                                                    bool *percentage_as_delta,
                                                    char *tilt,
                                                    bool *tilt_as_delta) {
  if (!s_abstract_action_shut_partially::get_expected(
          percentage, percentage_as_delta, tilt, tilt_as_delta)) {
    return false;
  }

  if (*percentage_as_delta) {
    *percentage *= -1;
  } else if (*percentage != -1) {
    *percentage = 100 - *percentage;
  }

  if (*tilt_as_delta) {
    *tilt *= -1;
  } else if (*tilt != -1) {
    *tilt = 100 - *tilt;
  }

  return true;
}

REGISTER_ACTION(s_worker_action_reveal_partially, ACTION_REVEAL_PARTIALLY);
