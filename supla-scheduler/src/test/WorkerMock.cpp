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

#include "WorkerMock.h"
#include <stdlib.h>
#include <string.h>
#include <cstddef>

WorkerMock::WorkerMock(queue *q) : s_abstract_worker(q) { action_param = NULL; }

WorkerMock::~WorkerMock() {
  if (action_param) {
    free(action_param);
    action_param = NULL;
  }
}

int WorkerMock::get_channel_func(void) { return 0; }

int WorkerMock::get_id(void) { return 0; }

int WorkerMock::get_retry_count(void) { return 0; }

bool WorkerMock::channel_group(void) { return false; }

const char *WorkerMock::get_action_param(void) { return action_param; }

void WorkerMock::set_action_param(const char *action_param) {
  if (this->action_param) {
    free(this->action_param);
    this->action_param = NULL;
  }

  if (action_param) {
    this->action_param = strndup(action_param, 255);
  }
}

bool WorkerMock::retry_when_fail(void) { return false; }

char WorkerMock::ipcc_get_opening_sensor_value(void) { return 0; }

bool WorkerMock::ipcc_set_char_value(char value) { return false; }

bool WorkerMock::ipcc_get_char_value(char *value) { return false; }

bool WorkerMock::ipcc_get_rgbw_value(int *color, char *color_brightness,
                                     char *brightness) {
  return false;
}
bool WorkerMock::ipcc_set_rgbw_value(int color, char color_brightness,
                                     char brightness) {
  return false;
}
bool WorkerMock::ipcc_get_valve_value(TValve_Value *value) { return false; }

bool WorkerMock::ipcc_set_digiglass_value(int active_bits, int mask) {
  return false;
}

bool WorkerMock::ipcc_get_digiglass_value(int *mask) { return false; }

char WorkerMock::ipcc_is_connected(void) { return 0; }
