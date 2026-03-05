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

#include "abstract_rs_value.h"

supla_abstract_rs_value::supla_abstract_rs_value(void)
    : supla_abstract_channel_value() {}

supla_abstract_rs_value::supla_abstract_rs_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {}

bool supla_abstract_rs_value::is_calibration_failed(void) {
  return get_flags() & RS_VALUE_FLAG_CALIBRATION_FAILED ? true : false;
}

bool supla_abstract_rs_value::is_calibration_lost(void) {
  return get_flags() & RS_VALUE_FLAG_CALIBRATION_LOST ? true : false;
}

bool supla_abstract_rs_value::is_motor_problem(void) {
  return get_flags() & RS_VALUE_FLAG_MOTOR_PROBLEM ? true : false;
}

bool supla_abstract_rs_value::is_calibration_in_progress(void) {
  return get_flags() & RS_VALUE_FLAG_CALIBRATION_IN_PROGRESS ? true : false;
}

bool supla_abstract_rs_value::is_any_error_set(void) {
  return ((get_flags() & RS_VALUE_FLAG_CALIBRATION_FAILED) ||
          (get_flags() & RS_VALUE_FLAG_CALIBRATION_LOST) ||
          (get_flags() & RS_VALUE_FLAG_MOTOR_PROBLEM))
             ? true
             : false;
}

nlohmann::json supla_abstract_rs_value::get_template_data(void) {
  nlohmann::json result = supla_abstract_channel_value::get_template_data();
  result["calibration_failed"] = is_calibration_failed();
  result["calibration_lost"] = is_calibration_lost();
  result["motor_problem"] = is_motor_problem();
  result["calibration_in_progress"] = is_calibration_in_progress();
  result["any_error_set"] = is_any_error_set();
  int i = get_position();
  if (i >= 0 && i <= 100) {
    result["shut"] = i;
    result["open"] = i < 100;
    result["closed"] = i == 100;
  }

  return result;
}

bool supla_abstract_rs_value::get_vbt_value(_vbt_var_name_e var_name,
                                            double *value) {
  switch (var_name) {
    case var_name_calibration_failed:
      *value = is_calibration_failed() ? 1 : 0;
      break;
    case var_name_calibration_lost:
      *value = is_calibration_lost() ? 1 : 0;
      break;
    case var_name_motor_problem:
      *value = is_motor_problem() ? 1 : 0;
      break;
    case var_name_calibration_in_progress:
      *value = is_calibration_in_progress() ? 1 : 0;
      break;
    case var_name_is_any_error_set:
      *value = is_any_error_set() ? 1 : 0;
      break;
    default:
      *value = get_position();
      break;
  }

  return true;
}
