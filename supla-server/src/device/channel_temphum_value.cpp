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

#include "channel_temphum_value.h"

#include <string.h>

supla_channel_temphum_value::supla_channel_temphum_value(
    bool temperature_only, char native_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(native_value) {
  this->temperature_only = temperature_only;
}

double supla_channel_temphum_value::get_temperature(void) {
  if (temperature_only) {
    double result = 0;
    memcpy(&result, native_value, sizeof(double));
    return result;
  }

  int n = 0;
  memcpy(&n, native_value, 4);
  return n / 1000.00;
}

double supla_channel_temphum_value::get_humidity(void) {
  if (temperature_only) {
    return -1;
  }
  int n = 0;
  memcpy(&n, &native_value[4], 4);
  return n / 1000.00;
}

void supla_channel_temphum_value::set_temperature(double temperature) {
  if (temperature < -273 || temperature > 1000) {
    temperature = -273;
  }

  if (temperature_only) {
    memcpy(native_value, &temperature, sizeof(double));
  } else {
    int n = temperature * 1000;
    memcpy(native_value, &n, 4);
  }
}

void supla_channel_temphum_value::set_humidity(double humidity) {
  if (!temperature_only) {
    if (humidity < -1 || humidity > 100) {
      humidity = -1;
    }

    int n = humidity * 1000;
    memcpy(&native_value[4], &n, 4);
  }
}

// static
bool supla_channel_temphum_value::significant_change(
    supla_channel_temphum_value *old_val,
    supla_channel_temphum_value *new_val) {
  return (new_val && !old_val) || (!new_val && old_val) ||
         ((int)(new_val->get_humidity() * 100) !=
              (int)(old_val->get_humidity() * 100) ||
          (int)(new_val->get_temperature() * 100) !=
              (int)(old_val->get_temperature() * 100));
}
