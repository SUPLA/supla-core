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

#include "channel_valve_value.h"

#include <string.h>

#include <string>

using std::map;
using std::string;

supla_channel_valve_value::supla_channel_valve_value(int func)
    : supla_abstract_channel_value() {
  this->func = func;
}

supla_channel_valve_value::supla_channel_valve_value(
    int func, const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {
  this->func = func;
}

supla_channel_valve_value::supla_channel_valve_value(
    int func, const TValve_Value *value) {
  this->func = func;
  memcpy(raw_value, value, sizeof(TValve_Value));
}

supla_abstract_channel_value *supla_channel_valve_value::copy(  // NOLINT
    void) const {                                               // NOLINT
  return new supla_channel_valve_value(func, raw_value);
}

void supla_channel_valve_value::get_valve_value(TValve_Value *value) {
  memcpy(value, raw_value, sizeof(TValve_Value));
}

const TValve_Value *supla_channel_valve_value::get_valve_value(void) {
  return (TValve_Value *)raw_value;
}

void supla_channel_valve_value::set_valve_value(TValve_Value *value) {
  memset(raw_value, 0, sizeof(raw_value));
  memcpy(raw_value, value, sizeof(TValve_Value));
}

// static
bool supla_channel_valve_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      return true;
  }

  return false;
}

bool supla_channel_valve_value::is_flooding(void) {
  return get_valve_value()->flags & SUPLA_VALVE_FLAG_FLOODING;
}

bool supla_channel_valve_value::is_manually_closed(void) {
  return get_valve_value()->flags & SUPLA_VALVE_FLAG_MANUALLY_CLOSED;
}

bool supla_channel_valve_value::is_motor_problem(void) {
  return get_valve_value()->flags & SUPLA_VALVE_FLAG_MOTOR_PROBLEM;
}

bool supla_channel_valve_value::is_closed(void) {
  return func == SUPLA_CHANNELFNC_VALVE_OPENCLOSE
             ? get_valve_value()->closed
             : get_valve_value()->closed_percent == 100;
}

nlohmann::json supla_channel_valve_value::get_template_data(void) {
  nlohmann::json result = supla_abstract_channel_value::get_template_data();
  result["flooding"] = is_flooding();
  result["manually_closed"] = is_manually_closed();
  result["motor_problem"] = is_motor_problem();
  result["closed"] = is_closed();
  result["open"] = !is_closed();

  if (func == SUPLA_CHANNELFNC_VALVE_PERCENTAGE) {
    int v = get_valve_value()->closed_percent;
    result["value"] = v;
  } else {
    result["value"] = is_closed();
  }

  // For backward compatibility
  result["is_closed_manually"] =
      get_valve_value()->closed &&
              (get_valve_value()->flags & SUPLA_VALVE_FLAG_MANUALLY_CLOSED)
          ? "Yes"
          : "No";

  return result;
}

bool supla_channel_valve_value::get_vbt_value(_vbt_var_name_e var_name,
                                              double *value) {
  switch (var_name) {
    case var_name_flooding:
      *value = is_flooding() ? 1 : 0;
      break;
    case var_name_manually_closed:
      *value = is_manually_closed() ? 1 : 0;
      break;
    case var_name_motor_problem:
      *value = is_motor_problem() ? 1 : 0;
      break;
    default:
      *value = is_closed() ? 1 : 0;
  }

  return true;
}
