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
    bool with_humidity, char native_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(native_value) {
  this->with_humidity = with_humidity;
}

supla_channel_temphum_value::supla_channel_temphum_value(bool with_humidity,
                                                         double temperature,
                                                         double humidity)
    : supla_channel_value() {
  this->with_humidity = with_humidity;
  set_temperature(temperature);
  set_humidity(humidity);
}

bool supla_channel_temphum_value::is_humidity_available(void) {
  return with_humidity;
}

double supla_channel_temphum_value::get_temperature(void) {
  if (!with_humidity) {
    double result = 0;
    memcpy(&result, native_value, sizeof(double));
    return result;
  }

  int n = 0;
  memcpy(&n, native_value, 4);
  return n / 1000.00;
}

double supla_channel_temphum_value::get_humidity(void) {
  if (!with_humidity) {
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

  if (with_humidity) {
    int n = temperature * 1000;
    memcpy(native_value, &n, 4);
  } else {
    memcpy(native_value, &temperature, sizeof(double));
  }
}

void supla_channel_temphum_value::set_humidity(double humidity) {
  if (with_humidity) {
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
