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

#include "jsonconfig/channel/temp_hum_config.h"

using std::map;
using std::string;

// static
int supla_channel_temphum_value::incorrect_temperature(void) { return -273; }

// static
int supla_channel_temphum_value::incorrect_humidity(void) { return -1; }

supla_channel_temphum_value::supla_channel_temphum_value(void)
    : supla_channel_value() {
  this->with_humidity = false;
  set_temperature(incorrect_temperature());
}

supla_channel_temphum_value::supla_channel_temphum_value(
    int channel_type, int func, char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {
  this->with_humidity =
      !((channel_type == SUPLA_CHANNELTYPE_THERMOMETERDS18B20 ||
         channel_type == SUPLA_CHANNELTYPE_THERMOMETER) &&
        func == SUPLA_CHANNELFNC_THERMOMETER);
}

supla_channel_temphum_value::supla_channel_temphum_value(
    bool with_humidity, char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {
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
    memcpy(&result, raw_value, sizeof(double));
    return result;
  }

  int n = 0;
  memcpy(&n, raw_value, 4);
  return n / 1000.00;
}

double supla_channel_temphum_value::get_humidity(void) {
  if (!with_humidity) {
    return incorrect_humidity();
  }
  int n = 0;
  memcpy(&n, &raw_value[4], 4);
  return n / 1000.00;
}

void supla_channel_temphum_value::set_temperature(double temperature) {
  if (temperature < incorrect_temperature()) {
    temperature = incorrect_temperature();
  }

  if (with_humidity) {
    int n = temperature * 1000;
    memcpy(raw_value, &n, 4);
  } else {
    memcpy(raw_value, &temperature, sizeof(double));
  }
}

void supla_channel_temphum_value::set_humidity(double humidity) {
  if (with_humidity) {
    if (humidity < incorrect_humidity() || humidity > 100) {
      humidity = incorrect_humidity();
    }

    int n = humidity * 1000;
    memcpy(&raw_value[4], &n, 4);
  }
}

bool supla_channel_temphum_value::is_differ(supla_channel_value *value,
                                            bool *significant_change) {
  bool result = supla_channel_value::is_differ(value, significant_change);
  if (result && significant_change) {
    supla_channel_temphum_value *temphum_val =
        dynamic_cast<supla_channel_temphum_value *>(value);

    *significant_change =
        !temphum_val || ((int)(get_humidity() * 100) !=
                             (int)(temphum_val->get_humidity() * 100) ||
                         (int)(get_temperature() * 100) !=
                             (int)(temphum_val->get_temperature() * 100));
  }

  return result;
}

void supla_channel_temphum_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {
  if (json_config) {
    temp_hum_config cfg(json_config);

    if (!cfg.is_adjustment_applied_by_device()) {
      set_temperature(get_temperature() + cfg.get_temperature_adjustment_dbl());
      set_humidity(get_humidity() + cfg.get_humidity_adjustment_dbl());
    }
  }
}

map<string, string> supla_channel_temphum_value::get_replacement_map(void) {
  map<string, string> result = supla_channel_value::get_replacement_map();

  char buffer[50] = {};
  snprintf(buffer, sizeof(buffer), "%.2f", get_temperature());

  result["temperature"] = buffer;
  if (is_humidity_available()) {
    snprintf(buffer, sizeof(buffer), "%.2f", get_humidity());
    result["humidity"] = buffer;
  }
  return result;
}

// static
bool supla_channel_temphum_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_HUMIDITY:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      return true;
  }

  return false;
}
