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

#include "controlling_the_gate_config.h"

#define NOA_MIN 1
#define NOA_MAX 5
#define NOA_DEFAULT 5

// static
const char controlling_the_gate_config::number_of_attempts_to_open_key[] =
    "numberOfAttemptsToOpen";

// static
const char controlling_the_gate_config::number_of_attempts_to_close_key[] =
    "numberOfAttemptsToClose";

// static
const char controlling_the_gate_config::state_verification_method_active_key[] =
    "stateVerificationMethodActive";

controlling_the_gate_config::controlling_the_gate_config(void)
    : channel_json_config() {}

controlling_the_gate_config::controlling_the_gate_config(
    channel_json_config *root)
    : channel_json_config(root) {}

int controlling_the_gate_config::get_number_of_attempts(const char *key) {
  cJSON *root = get_user_root();
  if (!root) {
    return NOA_DEFAULT;
  }

  cJSON *value = cJSON_GetObjectItem(root, key);
  if (value && cJSON_IsNumber(value)) {
    if (value->valueint >= NOA_MIN && value->valueint <= NOA_MAX) {
      return value->valueint;
    }
  }

  return NOA_DEFAULT;
}

int controlling_the_gate_config::get_number_of_attempts_to_open(void) {
  return get_number_of_attempts(number_of_attempts_to_open_key);
}

int controlling_the_gate_config::get_number_of_attempts_to_close(void) {
  return get_number_of_attempts(number_of_attempts_to_close_key);
}

bool controlling_the_gate_config::is_active_state_verification_method_enabled(
    void) {
  return get_bool(state_verification_method_active_key);
}
