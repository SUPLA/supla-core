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

#include "vbt_on_change_condition.h"

#include <math.h>

#include <map>
#include <string>

using std::map;
using std::string;

supla_vbt_on_change_condition::supla_vbt_on_change_condition(void) {
  value = 0;
  var_name = var_name_none;
  op = op_unknown;
  resume_op = op_unknown;
  resume_value = 0;
  paused = false;
  on_change = false;
}

supla_vbt_on_change_condition::~supla_vbt_on_change_condition(void) {}

double supla_vbt_on_change_condition::get_value(void) const { return value; }

_vbt_var_name_e supla_vbt_on_change_condition::get_var_name(void) const {
  return var_name;
}

_vbt_operator_e supla_vbt_on_change_condition::get_op(void) const { return op; }

_vbt_operator_e supla_vbt_on_change_condition::get_resume_op(void) const {
  return resume_op;
}

double supla_vbt_on_change_condition::get_resume_value(void) const {
  return resume_value;
}

bool supla_vbt_on_change_condition::is_paused(void) const { return paused; }

void supla_vbt_on_change_condition::apply_json_config(cJSON *json) {
  op = op_unknown;

  cJSON *root = cJSON_GetObjectItem(json, "on_change");
  on_change = root != nullptr;

  if (!root) {
    root = cJSON_GetObjectItem(json, "on_change_to");
    if (!root) {
      return;
    }
  }

  cJSON *name_json = cJSON_GetObjectItem(root, "name");
  if (name_json && cJSON_IsString(name_json)) {
    map<_vbt_var_name_e, string> names{
        {var_name_color, "color"},
        {var_name_color_brightness, "color_brightness"},
        {var_name_brightness, "brightness"},
        {var_name_temperature, "temperature"},
        {var_name_humidity, "humidity"},
        {var_name_flooding, "flooding"},
        {var_name_manually_closed, "manually_closed"},
        {var_name_voltage_avg, "voltage_avg"},
        {var_name_voltage1, "voltage1"},
        {var_name_voltage2, "voltage2"},
        {var_name_voltage3, "voltage3"},
        {var_name_current_sum, "current_sum"},
        {var_name_current1, "current1"},
        {var_name_current2, "current2"},
        {var_name_current3, "current3"},
        {var_name_power_active_sum, "power_active_sum"},
        {var_name_power_active1, "power_active1"},
        {var_name_power_active2, "power_active2"},
        {var_name_power_active3, "power_active3"},
        {var_name_power_reactive_sum, "power_reactive_sum"},
        {var_name_power_reactive1, "power_reactive1"},
        {var_name_power_reactive2, "power_reactive2"},
        {var_name_power_reactive3, "power_reactive3"},
        {var_name_power_apparent_sum, "power_apparent_sum"},
        {var_name_power_apparent1, "power_apparent1"},
        {var_name_power_apparent2, "power_apparent2"},
        {var_name_power_apparent3, "power_apparent3"},
        {var_name_fae1, "fae1"},
        {var_name_fae2, "fae2"},
        {var_name_fae3, "fae3"},
        {var_name_fae_sum, "fae_sum"},
        {var_name_fae_balanced, "fae_balanced"},
        {var_name_rae1, "rae1"},
        {var_name_rae2, "rae2"},
        {var_name_rae3, "rae3"},
        {var_name_rae_sum, "rae_sum"},
        {var_name_rae_balanced, "rae_balanced"},
        {var_name_counter, "counter"},
        {var_name_calculated_value, "calculated_value"},
        {var_name_heating, "heating"},
        {var_name_cooling, "cooling"},
        {var_name_heating_or_cooling, "heating_or_cooling"},
        {var_name_is_on, "is_on"},
        {var_name_is_any_error_set, "is_any_error_set"},
        {var_name_calibration_failed, "calibration_failed"},
        {var_name_calibration_lost, "calibration_lost"},
        {var_name_motor_problem, "motor_problem"},
        {var_name_calibration_in_progress, "calibration_in_progress"},
        {var_name_battery_level, "battery_level"},
        {var_name_battery_powered, "battery_powered"},
        {var_name_is_battery_cover_open, "is_battery_cover_open"},
        {var_name_thermometer_error, "thermometer_error"},
        {var_name_clock_error, "clock_error"},
        {var_name_invalid_value, "invalid_value"},
        {var_name_alarm, "alarm"},
        {var_name_warning, "warning"},
        {var_name_invalid_sensor_state, "invalid_sensor_state"},
        {var_name_sound_alarm_on, "sound_alarm_on"},
    };

    for (auto it = names.begin(); it != names.end(); ++it) {
      if (it->second == cJSON_GetStringValue(name_json)) {
        var_name = it->first;
        break;
      }
    }
  }

  if (on_change) {
    return;
  }

  if (!get_operator_and_value(root, &op, &value)) {
    op = op_unknown;
    return;
  }

  cJSON *resume_json = cJSON_GetObjectItem(root, "resume");
  if (resume_json &&
      !get_operator_and_value(resume_json, &resume_op, &resume_value)) {
    op = op_unknown;
    return;
  }
}

bool supla_vbt_on_change_condition::get_operator_and_value(cJSON *root,
                                                           _vbt_operator_e *op,
                                                           double *value) {
  map<_vbt_operator_e, string> ops{{op_eq, "eq"}, {op_ne, "ne"}, {op_gt, "gt"},
                                   {op_ge, "ge"}, {op_lt, "lt"}, {op_le, "le"}};

  cJSON *op_json = nullptr;

  for (auto it = ops.begin(); it != ops.end(); ++it) {
    op_json = cJSON_GetObjectItem(root, it->second.c_str());
    if (op_json) {
      *op = it->first;
      break;
    }
  }

  if (!op_json) {
    return false;
  }

  if (cJSON_IsNumber(op_json)) {
    *value = op_json->valuedouble;
  } else if (cJSON_IsBool(op_json)) {
    *value = cJSON_IsTrue(op_json) ? 1 : 0;
  } else if (cJSON_IsString(op_json)) {
    if (*op != op_eq && *op != op_ne) {
      return false;
    }

    string value_str = cJSON_GetStringValue(op_json);
    if (value_str == "hi" || value_str == "closed" || value_str == "on") {
      *value = 1;
    } else if (value_str == "lo" || value_str == "low" || value_str == "open" ||
               value_str == "off") {
      *value = 0;
    } else {
      return false;
    }
  }

  return true;
}

bool supla_vbt_on_change_condition::is_condition_met(_vbt_operator_e op,
                                                     double old_value,
                                                     double new_value,
                                                     double expected) {
  if (fabs(old_value - new_value) <= 0.000001) {
    return false;
  }

  return (op == op_eq && fabs(new_value - expected) <= 0.000001) ||
         (op == op_ne && fabs(new_value - expected) > 0.000001) ||
         (op == op_gt && new_value > expected && old_value <= expected) ||
         (op == op_lt && new_value < expected && old_value >= expected) ||
         (op == op_ge && new_value >= expected && old_value < expected) ||
         (op == op_le && new_value <= expected && old_value > expected);
}

bool supla_vbt_on_change_condition::is_condition_met(double old_value,
                                                     double new_value) {
  if (on_change) {
    return fabs(new_value - old_value) > 0.000001;
  }

  if (paused) {
    if (is_condition_met(resume_op, old_value, new_value, resume_value)) {
      paused = false;
    }
  } else {
    bool result = is_condition_met(op, old_value, new_value, value);

    if (result && resume_op != op_unknown) {
      paused = true;
    }

    return result;
  }

  return false;
}

bool supla_vbt_on_change_condition::is_condition_met(
    supla_vbt_value *old_value, supla_vbt_value *new_value) {
  if (!on_change && op == op_unknown) {
    return false;
  }

  double oldv = 0;
  double newv = 0;

  if (!old_value || !old_value->get_vbt_value(var_name, &oldv)) {
    return false;
  }

  if (!new_value || !new_value->get_vbt_value(var_name, &newv)) {
    return false;
  }

  return is_condition_met(oldv, newv);
}

bool supla_vbt_on_change_condition::operator==(
    const supla_vbt_on_change_condition &cnd) const {
  return value == cnd.value && var_name == cnd.var_name && op == cnd.op &&
         resume_op == cnd.resume_op && resume_value == cnd.resume_value;
}
