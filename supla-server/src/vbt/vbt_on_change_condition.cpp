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

#include <map>
#include <string>

#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
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
}

supla_vbt_on_change_condition::~supla_vbt_on_change_condition(void) {}

void supla_vbt_on_change_condition::apply_json_config(cJSON *json) {
  op = op_unknown;
  cJSON *root = cJSON_GetObjectItem(json, "on_change_to");
  if (!root) {
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

  for (auto it = ops.rbegin(); it != ops.rend(); ++it) {
    cJSON *op_json = cJSON_GetObjectItem(root, it->second.c_str());
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
    *result =
        dynamic_cast<supla_channel_rs_value *>(value)->get_rs_value()->position;
    return true;
  }

  if (dynamic_cast<supla_channel_temphum_value *>(value)) {
    supla_channel_temphum_value *temphum =
        dynamic_cast<supla_channel_temphum_value *>(value);

    switch (var_name) {
      case var_name_temperature:
        *result = temphum->get_temperature();
        break;
      case var_name_humidity:
        if (!temphum->is_humidity_available()) {
          return false;
        }
        *result = temphum->get_humidity();
        break;
      default:
        return false;
    }

    return true;
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
        return vv->get_valve_value()->closed ? 1 : 0;
    }

    return true;
  }

  return false;
}

bool supla_vbt_on_change_condition::is_condition_met(_vbt_operator_e op,
                                                     double old_value,
                                                     double new_value,
                                                     double expected) {
  if (old_value == new_value) {
    return false;
  }

  return (op == op_eq && new_value == expected) ||
         (op == op_ne && new_value != expected) ||
         (op == op_gt && new_value > expected && old_value <= expected) ||
         (op == op_lt && new_value < expected && old_value >= expected) ||
         (op == op_ge && new_value >= expected && old_value < expected) ||
         (op == op_le && new_value <= expected && old_value > expected);
}

bool supla_vbt_on_change_condition::is_condition_met(double old_value,
                                                     double new_value) {
  bool result = is_condition_met(op, old_value, new_value, value);

  if (paused) {
    if (result) {
      paused = false;
    }
    return false;
  }

  if (result && resume_op != op_unknown) {
    paused = true;
  }

  return result;
}

bool supla_vbt_on_change_condition::is_condition_met(
    supla_channel_value *old_value, supla_channel_value *new_value) {
  if (op == op_unknown) {
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
