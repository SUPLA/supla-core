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

#include "device/extended_value/channel_and_timer_state_extended_value.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/extended_value/channel_state_extended_value.h"
#include "device/extended_value/channel_thermostat_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_fb_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_general_purpose_base_value.h"
#include "device/value/channel_hvac_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_valve_value.h"

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

bool supla_vbt_on_change_condition::get_number(supla_channel_value *value,
                                               double *result) {
  if (dynamic_cast<supla_channel_binary_sensor_value *>(value)) {
    *result = dynamic_cast<supla_channel_binary_sensor_value *>(value)->is_hi()
                  ? 1.0
                  : 0.0;
    return true;
  }

  if (dynamic_cast<supla_channel_floating_point_sensor_value *>(value)) {
    *result = dynamic_cast<supla_channel_floating_point_sensor_value *>(value)
                  ->get_value();
    return true;
  }

  if (dynamic_cast<supla_channel_onoff_value *>(value)) {
    *result = dynamic_cast<supla_channel_onoff_value *>(value)->is_on() ? 1 : 0;
    return true;
  }

  if (dynamic_cast<supla_channel_rgbw_value *>(value)) {
    supla_channel_rgbw_value *rgbw =
        dynamic_cast<supla_channel_rgbw_value *>(value);

    switch (var_name) {
      case var_name_color:
        *result = rgbw->get_color();
        break;
      case var_name_color_brightness:
        *result = rgbw->get_color_brightness();
        break;
      case var_name_brightness:
        *result = rgbw->get_brightness();
        break;
      default:
        return false;
    }
    return true;
  }

  if (dynamic_cast<supla_channel_rs_value *>(value)) {
    supla_channel_rs_value *rs = dynamic_cast<supla_channel_rs_value *>(value);

    switch (var_name) {
      case var_name_calibration_failed:
        *result = rs->get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_FAILED
                      ? 1
                      : 0;
        break;
      case var_name_calibration_lost:
        *result =
            rs->get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_LOST ? 1 : 0;
        break;
      case var_name_motor_problem:
        *result =
            rs->get_rs_value()->flags & RS_VALUE_FLAG_MOTOR_PROBLEM ? 1 : 0;
        break;
      case var_name_calibration_in_progress:
        *result =
            rs->get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_IN_PROGRESS
                ? 1
                : 0;
        break;
      case var_name_is_any_error_set:
        *result =
            ((rs->get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_FAILED) ||
             (rs->get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_LOST) ||
             (rs->get_rs_value()->flags & RS_VALUE_FLAG_MOTOR_PROBLEM))
                ? 1
                : 0;
        break;
      default:
        *result = rs->get_rs_value()->position;
        break;
    }

    return true;
  }

  if (dynamic_cast<supla_channel_fb_value *>(value)) {
    *result =
        dynamic_cast<supla_channel_fb_value *>(value)->get_fb_value()->position;
    return true;
  }

  if (dynamic_cast<supla_channel_temphum_value *>(value)) {
    supla_channel_temphum_value *temphum =
        dynamic_cast<supla_channel_temphum_value *>(value);

    switch (var_name) {
      case var_name_temperature: {
        double temperature = temphum->get_temperature();
        if (temperature >
            supla_channel_temphum_value::incorrect_temperature()) {
          *result = temperature;
          return true;
        }
      } break;
      case var_name_humidity:
        if (temphum->is_humidity_available()) {
          double humidity = temphum->get_humidity();
          if (humidity > supla_channel_temphum_value::incorrect_humidity()) {
            *result = temphum->get_humidity();
            return true;
          }
        }
        break;
      default:
        break;
    }

    return false;
  }

  if (dynamic_cast<supla_channel_valve_value *>(value)) {
    supla_channel_valve_value *vv =
        dynamic_cast<supla_channel_valve_value *>(value);

    switch (var_name) {
      case var_name_flooding:
        *result =
            (vv->get_valve_value()->flags & SUPLA_VALVE_FLAG_FLOODING) ? 1 : 0;
        break;
      case var_name_manually_closed:
        *result =
            (vv->get_valve_value()->flags & SUPLA_VALVE_FLAG_MANUALLY_CLOSED)
                ? 1
                : 0;
        break;
      default:
        *result = vv->get_valve_value()->closed ? 1 : 0;
    }

    return true;
  }

  if (dynamic_cast<supla_channel_hvac_value *>(value)) {
    supla_channel_hvac_value *hvac =
        dynamic_cast<supla_channel_hvac_value *>(value);

    switch (var_name) {
      case var_name_heating:
        *result = hvac->is_heating() ? 1 : 0;
        break;
      case var_name_cooling:
        *result = hvac->is_cooling() ? 1 : 0;
        break;
      case var_name_heating_or_cooling:
        *result = hvac->is_heating() || hvac->is_cooling() ? 1 : 0;
        break;
      case var_name_is_on:
        *result = hvac->is_on() ? 1 : 0;
        break;
      case var_name_is_any_error_set:
        *result = hvac->is_any_error_set() ? 1 : 0;
        break;
      case var_name_is_battery_cover_open:
        *result = hvac->is_battery_cover_open() ? 1 : 0;
        break;
      case var_name_thermometer_error:
        *result = hvac->thermometer_error() ? 1 : 0;
        break;
      case var_name_clock_error:
        *result = hvac->clock_error() ? 1 : 0;
        break;
      default:
        return false;
    }

    return true;
  }

  if (dynamic_cast<supla_channel_general_purpose_base_value *>(value)) {
    *result = dynamic_cast<supla_channel_general_purpose_base_value *>(value)
                  ->get_value();
    return true;
  }

  return false;
}

bool supla_vbt_on_change_condition::get_number(
    supla_channel_extended_value *value, double *result) {
  if (dynamic_cast<supla_channel_em_extended_value *>(value)) {
    supla_channel_em_extended_value *em =
        dynamic_cast<supla_channel_em_extended_value *>(value);

    switch (var_name) {
      case var_name_voltage_avg:
        *result = em->get_voltage_avg();
        break;
      case var_name_voltage1:
        *result = em->get_voltage(1);
        break;
      case var_name_voltage2:
        *result = em->get_voltage(2);
        break;
      case var_name_voltage3:
        *result = em->get_voltage(3);
        break;
      case var_name_current_sum:
        *result = em->get_current_sum();
        break;
      case var_name_current1:
        *result = em->get_current(1);
        break;
      case var_name_current2:
        *result = em->get_current(2);
        break;
      case var_name_current3:
        *result = em->get_current(3);
        break;
      case var_name_power_active_sum:
        *result = em->get_power_active_sum();
        break;
      case var_name_power_active1:
        *result = em->get_power_active(1);
        break;
      case var_name_power_active2:
        *result = em->get_power_active(2);
        break;
      case var_name_power_active3:
        *result = em->get_power_active(3);
        break;
      case var_name_power_reactive_sum:
        *result = em->get_power_reactive_sum();
        break;
      case var_name_power_reactive1:
        *result = em->get_power_reactive(1);
        break;
      case var_name_power_reactive2:
        *result = em->get_power_reactive(2);
        break;
      case var_name_power_reactive3:
        *result = em->get_power_reactive(3);
        break;
      case var_name_power_apparent_sum:
        *result = em->get_power_apparent_sum();
        break;
      case var_name_power_apparent1:
        *result = em->get_power_apparent(1);
        break;
      case var_name_power_apparent2:
        *result = em->get_power_apparent(2);
        break;
      case var_name_power_apparent3:
        *result = em->get_power_apparent(3);
        break;
      case var_name_fae1:
        *result = em->get_fae(1);
        break;
      case var_name_fae2:
        *result = em->get_fae(2);
        break;
      case var_name_fae3:
        *result = em->get_fae(3);
        break;
      case var_name_fae_sum:
        *result = em->get_fae_sum();
        break;
      case var_name_fae_balanced:
        *result = em->get_fae_balanced();
        break;
      case var_name_rae1:
        *result = em->get_rae(1);
        break;
      case var_name_rae2:
        *result = em->get_rae(2);
        break;
      case var_name_rae3:
        *result = em->get_rae(3);
        break;
      case var_name_rae_sum:
        *result = em->get_rae_sum();
        break;
      case var_name_rae_balanced:
        *result = em->get_rae_balanced();
        break;
      default:
        return false;
    }

    return true;
  } else if (dynamic_cast<supla_channel_ic_extended_value *>(value)) {
    supla_channel_ic_extended_value *ic =
        dynamic_cast<supla_channel_ic_extended_value *>(value);

    switch (var_name) {
      case var_name_counter:
        *result = ic->get_counter();
        break;
      case var_name_calculated_value:
        *result = ic->get_calculated_value_dbl();
        break;
      default:
        return false;
    }

    return true;
  } else if (dynamic_cast<supla_channel_thermostat_extended_value *>(value)) {
    supla_channel_thermostat_extended_value *thev =
        dynamic_cast<supla_channel_thermostat_extended_value *>(value);

    // Currently, only heatpol uses this, so we do not need to check the channel
    // function.

    supla_channel_hp_thermostat_ev_decorator decorator(thev);

    switch (var_name) {
      case var_name_heating:
      case var_name_is_on:
        *result = decorator.is_heating() ? 1 : 0;
        break;
      default:
        return false;
    }

    return true;
  } else if (dynamic_cast<supla_channel_state_extended_value *>(value) ||
             dynamic_cast<supla_channel_and_timer_state_extended_value *>(
                 value)) {
    TChannelState_ExtendedValue raw = {};

    if (dynamic_cast<supla_channel_state_extended_value *>(value)) {
      dynamic_cast<supla_channel_state_extended_value *>(value)->get_raw_value(
          &raw);
    } else {
      dynamic_cast<supla_channel_and_timer_state_extended_value *>(value)
          ->get_raw_value(&raw);
    }

    switch (var_name) {
      case var_name_battery_level:
        if (raw.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL) {
          *result = raw.BatteryLevel;
          return true;
        }
        break;
      case var_name_battery_powered:
        if (raw.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED) {
          *result = raw.BatteryPowered ? 1 : 0;
          return true;
        }
        break;
      default:
        return false;
    }
  }

  return false;
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
    supla_channel_value *old_value, supla_channel_value *new_value) {
  if (!on_change && op == op_unknown) {
    return false;
  }

  double oldv = 0;
  double newv = 0;

  if (!get_number(old_value, &oldv)) {
    return false;
  }

  if (!get_number(new_value, &newv)) {
    return false;
  }

  return is_condition_met(oldv, newv);
}

bool supla_vbt_on_change_condition::is_condition_met(
    supla_channel_extended_value *old_value,
    supla_channel_extended_value *new_value) {
  if (!on_change && op == op_unknown) {
    return false;
  }

  double oldv = 0;
  double newv = 0;

  if (!get_number(old_value, &oldv)) {
    return false;
  }

  if (!get_number(new_value, &newv)) {
    return false;
  }

  return is_condition_met(oldv, newv);
}

bool supla_vbt_on_change_condition::operator==(
    const supla_vbt_on_change_condition &cnd) const {
  return value == cnd.value && var_name == cnd.var_name && op == cnd.op &&
         resume_op == cnd.resume_op && resume_value == cnd.resume_value;
}
