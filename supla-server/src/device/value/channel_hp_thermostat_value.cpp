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

#include "channel_hp_thermostat_value.h"

#include <string.h>

using std::string;

supla_channel_hp_thermostat_value::supla_channel_hp_thermostat_value(void)
    : supla_abstract_channel_value() {}

supla_channel_hp_thermostat_value::supla_channel_hp_thermostat_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {}

supla_abstract_channel_value *
supla_channel_hp_thermostat_value::copy(  // NOLINT
    void) const {                         // NOLINT
  return new supla_channel_hp_thermostat_value(raw_value);
}

bool supla_channel_hp_thermostat_value::is_on(void) {
  return ((TThermostat_Value *)raw_value)->IsOn > 0;
}

double supla_channel_hp_thermostat_value::get_measured_temperature(void) {
  return ((TThermostat_Value *)raw_value)->MeasuredTemperature * 0.01;
}

double supla_channel_hp_thermostat_value::get_preset_temperature(void) {
  return ((TThermostat_Value *)raw_value)->PresetTemperature * 0.01;
}

string supla_channel_hp_thermostat_value::get_measured_temperature_str(void) {
  char result[20] = {};
  snprintf(result, sizeof(result), "%.2f", get_measured_temperature());
  return result;
}

string supla_channel_hp_thermostat_value::get_preset_temperature_str(void) {
  char result[20] = {};
  snprintf(result, sizeof(result), "%.2f", get_preset_temperature());
  return result;
}

// static
bool supla_channel_hp_thermostat_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      return true;
  }

  return false;
}
