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

#ifndef ABSTRACT_VALUE_BASED_TRIGGER_CONDITION_H_
#define ABSTRACT_VALUE_BASED_TRIGGER_CONDITION_H_

#include <string>

#include "device/value/channel_value.h"
#include "json/cJSON.h"

enum _vbt_var_name_e {
  var_name_none,
  var_name_color,
  var_name_color_brightness,
  var_name_brightness,
  var_name_temperature,
  var_name_humidity,
  var_name_flooding,
  var_name_manually_closed,
  var_name_voltage,
  var_name_current,
  var_name_power_active,
  var_name_power_reactive,
  var_name_power_apparent
};

enum _vbt_operator_e { op_unknown, op_eq, op_ne, op_le, op_lt, op_gt, op_ge };

class supla_vbt_on_change_condition {
 private:
  double value;
  _vbt_var_name_e var_name;
  _vbt_operator_e op;
  _vbt_operator_e resume_op;
  double resume_value;
  bool paused;

  bool get_operator_and_value(cJSON *root, _vbt_operator_e *op, double *value);

  bool get_number(supla_channel_value *value, double *result);
  bool is_condition_met(_vbt_operator_e op, double old_value, double new_value,
                        double expected);
  bool is_condition_met(double old_value, double new_value);

 public:
  supla_vbt_on_change_condition(void);
  virtual ~supla_vbt_on_change_condition(void);

  void apply_json_config(cJSON *json);

  double get_value(void) const;
  _vbt_var_name_e get_var_name(void) const;
  _vbt_operator_e get_op(void) const;
  _vbt_operator_e get_resume_op(void) const;
  double get_resume_value(void) const;
  bool is_paused(void) const;

  virtual bool is_condition_met(supla_channel_value *old_value,
                                supla_channel_value *new_value);

  bool operator==(const supla_vbt_on_change_condition &cnd) const;
};

#endif /* ABSTRACT_VALUE_BASED_TRIGGER_CONDITION_H_ */
