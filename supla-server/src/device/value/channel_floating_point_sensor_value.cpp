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

#include "channel_floating_point_sensor_value.h"

#include <string.h>

supla_channel_floating_point_sensor_value::
    supla_channel_floating_point_sensor_value(int func)
    : supla_abstract_channel_value() {
  this->func = func;
}

supla_channel_floating_point_sensor_value::
    supla_channel_floating_point_sensor_value(
        int func, const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {
  this->func = func;
}

supla_channel_floating_point_sensor_value::
    supla_channel_floating_point_sensor_value(int func, double value)
    : supla_abstract_channel_value() {
  this->func = func;
  set_value(value);
}

supla_abstract_channel_value *
supla_channel_floating_point_sensor_value::copy(  // NOLINT
    void) const {                                 // NOLINT
  return new supla_channel_floating_point_sensor_value(func, raw_value);
}

void supla_channel_floating_point_sensor_value::set_value(double value) {
  memcpy(raw_value, &value, sizeof(double));
}

double supla_channel_floating_point_sensor_value::get_value(void) {
  double result = 0;
  memcpy(&result, raw_value, sizeof(double));
  return result;
}

// static
bool supla_channel_floating_point_sensor_value::is_function_supported(
    int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
    case SUPLA_CHANNELFNC_WINDSENSOR:
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
    case SUPLA_CHANNELFNC_RAINSENSOR:
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      return true;
  }

  return false;
}

nlohmann::json supla_channel_floating_point_sensor_value::get_template_data(
    void) {
  nlohmann::json result = supla_abstract_channel_value::get_template_data();
  result["value"] = get_value();
  return result;
}

bool supla_channel_floating_point_sensor_value::get_vbt_value(
    _vbt_var_name_e var_name, double *value) {
  double v = get_value();
  if (v >= 0 || func != SUPLA_CHANNELFNC_DISTANCESENSOR) {
    *value = v;
    return true;
  }
  return false;
}
