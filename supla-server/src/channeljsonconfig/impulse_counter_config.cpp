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

#include "impulse_counter_config.h"

#include "srpc.h"

#define INITIAL_VALUE_MAX 100000000
#define IMPULSES_PER_UNIT_MAX 1000000

// static
const char impulse_counter_config::initial_value_key[] = "initialValue";

// static
const char impulse_counter_config::impulses_per_unit_key[] = "impulsesPerUnit";

impulse_counter_config::impulse_counter_config(void) : channel_json_config() {}

impulse_counter_config::impulse_counter_config(channel_json_config *root)
    : channel_json_config(root) {}

double impulse_counter_config::get_double_value(const char *key, double max) {
  cJSON *root = get_user_root();
  if (!root) {
    return 0;
  }

  cJSON *initial_value = cJSON_GetObjectItem(root, key);
  if (initial_value && cJSON_IsNumber(initial_value)) {
    if (initial_value->valuedouble > max) {
      return max;
    }
    return initial_value->valuedouble;
  }

  return 0;
}

double impulse_counter_config::get_initial_value(void) {
  return get_double_value(initial_value_key, INITIAL_VALUE_MAX);
}

unsigned int impulse_counter_config::get_impulses_per_unit(void) {
  return get_double_value(impulses_per_unit_key, IMPULSES_PER_UNIT_MAX);
}

void impulse_counter_config::add(TDS_ImpulseCounter_Value *value) {
  if (!value) {
    return;
  }

  unsigned _supla_int64_t left = -1 - value->counter;
  double initial_value = get_initial_value();
  int impulses_per_unit = get_impulses_per_unit();

  if (initial_value == 0 || impulses_per_unit == 0) {
    return;
  }

  unsigned _supla_int64_t _initial_value = initial_value * impulses_per_unit;

  if (_initial_value > left) {
    _initial_value = left;
  }

  value->counter += _initial_value;
}
