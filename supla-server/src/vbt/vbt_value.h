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

#ifndef ABSTRACT_VALUE_BASED_TRIGGER_VALUE_H_
#define ABSTRACT_VALUE_BASED_TRIGGER_VALUE_H_

#include <map>
#include <string>

enum _vbt_var_name_e {
  var_name_none,
  var_name_color,
  var_name_color_brightness,
  var_name_brightness,
  var_name_temperature,
  var_name_humidity,
  var_name_flooding,
  var_name_manually_closed,
  var_name_voltage_avg,
  var_name_voltage1,
  var_name_voltage2,
  var_name_voltage3,
  var_name_current_sum,
  var_name_current1,
  var_name_current2,
  var_name_current3,
  var_name_power_active_sum,
  var_name_power_active1,
  var_name_power_active2,
  var_name_power_active3,
  var_name_power_reactive_sum,
  var_name_power_reactive1,
  var_name_power_reactive2,
  var_name_power_reactive3,
  var_name_power_apparent_sum,
  var_name_power_apparent1,
  var_name_power_apparent2,
  var_name_power_apparent3,
  var_name_fae1,
  var_name_fae2,
  var_name_fae3,
  var_name_fae_sum,
  var_name_fae_balanced,
  var_name_rae1,
  var_name_rae2,
  var_name_rae3,
  var_name_rae_sum,
  var_name_rae_balanced,
  var_name_counter,
  var_name_calculated_value,
  var_name_heating,
  var_name_cooling,
  var_name_heating_or_cooling,
  var_name_is_on,
  var_name_is_any_error_set,
  var_name_calibration_failed,
  var_name_calibration_lost,
  var_name_motor_problem,
  var_name_calibration_in_progress,
  var_name_battery_level,
  var_name_battery_powered,
  var_name_is_battery_cover_open,
  var_name_thermometer_error,
  var_name_clock_error,
  var_name_invalid_value,
  var_name_alarm,
  var_name_sound_alarm_on,
  var_name_warning,
  var_name_invalid_sensor_state,
};

class supla_vbt_value {
 public:
  supla_vbt_value(void);
  virtual ~supla_vbt_value(void);
  virtual bool get_vbt_value(_vbt_var_name_e var_name, double *value);
  virtual std::map<std::string, std::string> get_replacement_map(void);
};

#endif /* ABSTRACT_VALUE_BASED_TRIGGER_VALUE_H_ */
