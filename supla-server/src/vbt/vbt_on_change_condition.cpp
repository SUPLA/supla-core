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
  text_value.clear();
  var_name = var_name_none;
  op = op_unknown;
  resume_op = op_unknown;
  resume_value = 0;
  paused = false;
  on_change = false;
  text_comparison = false;
  duration_sec = 0;
  saved_old_value = 0;
  saved_old_text.clear();
  condition_met_at = {};
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

int supla_vbt_on_change_condition::get_duration_sec(void) const {
  return duration_sec;
}

bool supla_vbt_on_change_condition::is_paused(void) const { return paused; }

void supla_vbt_on_change_condition::apply_json_config(cJSON *json) {
  op = op_unknown;
  text_value.clear();
  text_comparison = false;
  saved_old_text.clear();

  cJSON *root = cJSON_GetObjectItem(json, "on_change");
  on_change = root != nullptr;

  if (!root) {
    root = cJSON_GetObjectItem(json, "on_change_to");
    if (!root) {
      return;
    }
  }

  cJSON *duration_json = cJSON_GetObjectItem(root, "duration_sec");
  if (cJSON_IsNumber(duration_json)) {
    duration_sec = cJSON_GetNumberValue(duration_json);
  }

  cJSON *name_json = cJSON_GetObjectItem(root, "name");
  if (cJSON_IsString(name_json)) {
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
        {var_name_connected, "connected"},
        {var_name_white_temperature, "white_temperature"},
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

  if (!get_operator_and_value(root, &op, &value, &text_value,
                              &text_comparison)) {
    op = op_unknown;
    return;
  }

  cJSON *resume_json = cJSON_GetObjectItem(root, "resume");
  std::string resume_text_value;
  bool resume_text_comparison = false;
  if (resume_json &&
      !get_operator_and_value(resume_json, &resume_op, &resume_value,
                              &resume_text_value,
                              &resume_text_comparison)) {
    op = op_unknown;
    return;
  }

  if (resume_text_comparison) {
    op = op_unknown;
    return;
  }
}

bool supla_vbt_on_change_condition::get_operator_and_value(cJSON *root,
                                                           _vbt_operator_e *op,
                                                           double *value,
                                                           std::string *text_value,
                                                           bool *text_comparison) {
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
    *text_comparison = false;
  } else if (cJSON_IsBool(op_json)) {
    *value = cJSON_IsTrue(op_json) ? 1 : 0;
    *text_comparison = false;
  } else if (cJSON_IsString(op_json)) {
    if (*op != op_eq && *op != op_ne) {
      return false;
    }

    string value_str = cJSON_GetStringValue(op_json);
    if (value_str == "hi" || value_str == "closed" || value_str == "on") {
      *value = 1;
      *text_comparison = false;
    } else if (value_str == "lo" || value_str == "low" || value_str == "open" ||
               value_str == "off") {
      *value = 0;
      *text_comparison = false;
    } else {
      *text_value = value_str;
      *text_comparison = true;
    }
  } else {
    return false;
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

bool supla_vbt_on_change_condition::is_condition_met(
    _vbt_operator_e op, const std::string &old_value,
    const std::string &new_value, const std::string &expected) {
  if (old_value == new_value) {
    return false;
  }

  return (op == op_eq && new_value == expected) ||
         (op == op_ne && new_value != expected);
}

bool supla_vbt_on_change_condition::is_condition_met(
    double old_value, double new_value, _supla_int64_t *milliseconds_left) {
  if (milliseconds_left) {
    *milliseconds_left = 0;
  }

  if (on_change) {
    if (fabs(new_value - old_value) > 0.000001) {
      if (duration_sec > 0) {
        struct timeval now;
        gettimeofday(&now, nullptr);
        condition_met_at = now;
        condition_met_at.tv_sec += duration_sec;

        if (milliseconds_left) {
          *milliseconds_left = ms_left(now);
        }

        return true;
      }

      condition_met_at = {};
      return true;
    }

    condition_met_at = {};
    return false;
  }

  if (paused) {
    if (is_condition_met(resume_op, old_value, new_value, resume_value)) {
      paused = false;
    }
  } else if (is_condition_met(
                 op, condition_met_at.tv_sec ? saved_old_value : old_value,
                 new_value, value)) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    if (duration_sec > 0) {
      if (!condition_met_at.tv_sec) {
        saved_old_value = old_value;
        condition_met_at = now;
        condition_met_at.tv_sec += duration_sec;
      }

      if (milliseconds_left) {
        *milliseconds_left = ms_left(now);
      }
    } else if (resume_op != op_unknown) {
      paused = true;
    }

    return true;
  }

  condition_met_at = {};
  return false;
}

bool supla_vbt_on_change_condition::is_condition_met(
    const std::string &old_value, const std::string &new_value,
    _supla_int64_t *milliseconds_left) {
  if (milliseconds_left) {
    *milliseconds_left = 0;
  }

  if (on_change) {
    if (old_value != new_value) {
      if (duration_sec > 0) {
        struct timeval now;
        gettimeofday(&now, nullptr);
        condition_met_at = now;
        condition_met_at.tv_sec += duration_sec;

        if (milliseconds_left) {
          *milliseconds_left = ms_left(now);
        }

        return true;
      }

      condition_met_at = {};
      return true;
    }

    condition_met_at = {};
    return false;
  }

  if (paused) {
    return false;
  } else if (is_condition_met(op, condition_met_at.tv_sec ? saved_old_text : old_value,
                              new_value, text_value)) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    if (duration_sec > 0) {
      if (!condition_met_at.tv_sec) {
        saved_old_text = old_value;
        condition_met_at = now;
        condition_met_at.tv_sec += duration_sec;
      }

      if (milliseconds_left) {
        *milliseconds_left = ms_left(now);
      }
    }

    return true;
  }

  condition_met_at = {};
  return false;
}

bool supla_vbt_on_change_condition::is_condition_met(
    supla_vbt_value *old_value, supla_vbt_value *new_value,
    _supla_int64_t *milliseconds_left) {
  if (!on_change && op == op_unknown) {
    return false;
  }

  if (!old_value || !new_value) {
    return false;
  }

  double oldv = 0;
  double newv = 0;

  bool old_numeric = old_value->get_vbt_value(var_name, &oldv);
  bool new_numeric = new_value->get_vbt_value(var_name, &newv);

  if (!old_numeric || !new_numeric) {
    std::string old_text;
    std::string new_text;
    if (!old_value->get_vbt_text_value(var_name, &old_text) ||
        !new_value->get_vbt_text_value(var_name, &new_text)) {
      return false;
    }

    return is_condition_met(old_text, new_text, milliseconds_left);
  }

  return is_condition_met(oldv, newv, milliseconds_left);
}

bool supla_vbt_on_change_condition::is_condition_met(
    supla_vbt_value *old_value, supla_vbt_value *new_value) {
  return is_condition_met(old_value, new_value, nullptr);
}

_supla_int64_t supla_vbt_on_change_condition::ms_left(
    const struct timeval &now) {
  return ((condition_met_at.tv_sec * 1000000LL + condition_met_at.tv_usec) -
          (now.tv_sec * 1000000LL + now.tv_usec)) /
         1000;
}

bool supla_vbt_on_change_condition::is_condition_met(
    _supla_int64_t *milliseconds_left) {
  if (condition_met_at.tv_sec) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    *milliseconds_left = ms_left(now);

    if (*milliseconds_left <= 0) {
      if (resume_op != op_unknown) {
        paused = true;
      }

      condition_met_at = {};
      return true;
    }
  } else {
    *milliseconds_left = 0;
  }

  return false;
}

bool supla_vbt_on_change_condition::operator==(
    const supla_vbt_on_change_condition &cnd) const {
  return value == cnd.value && var_name == cnd.var_name && op == cnd.op &&
         resume_op == cnd.resume_op && resume_value == cnd.resume_value &&
         text_comparison == cnd.text_comparison && text_value == cnd.text_value &&
         duration_sec == cnd.duration_sec && on_change == cnd.on_change;
}
