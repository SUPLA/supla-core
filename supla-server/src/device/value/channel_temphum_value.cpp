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

#include <cstdio>
#include <string>

#include "jsonconfig/channel/temp_hum_config.h"

using std::map;
using std::string;

// static
int supla_channel_temphum_value::incorrect_temperature(void) { return -273; }

// static
int supla_channel_temphum_value::incorrect_humidity(void) { return -1; }

supla_channel_temphum_value::supla_channel_temphum_value(int type, int func)
    : supla_abstract_channel_value() {
  this->type = type;
  this->func = func;
  set_temperature(incorrect_temperature());
}

supla_channel_temphum_value::supla_channel_temphum_value(
    int type, int func, const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {
  this->type = type;
  this->func = func;
}

supla_channel_temphum_value::supla_channel_temphum_value(int type, int func,
                                                         double temperature,
                                                         double humidity)
    : supla_abstract_channel_value() {
  this->type = type;
  this->func = func;

  set_temperature(temperature);
  set_humidity(humidity);
}

supla_abstract_channel_value *supla_channel_temphum_value::copy(  // NOLINT
    void) const {                                                 // NOLINT
  return new supla_channel_temphum_value(type, func, raw_value);
}

bool supla_channel_temphum_value::is_humidity_available(void) {
  return !((type == SUPLA_CHANNELTYPE_THERMOMETERDS18B20 ||
            type == SUPLA_CHANNELTYPE_THERMOMETER) &&
           func == SUPLA_CHANNELFNC_THERMOMETER);
}

double supla_channel_temphum_value::get_temperature(void) {
  if (!is_humidity_available()) {
    double result = 0;
    memcpy(&result, raw_value, sizeof(double));
    return result;
  }

  int n = 0;
  memcpy(&n, raw_value, 4);
  return n / 1000.00;
}

double supla_channel_temphum_value::get_humidity(void) {
  if (!is_humidity_available()) {
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

  if (is_humidity_available()) {
    int n = temperature * 1000;
    memcpy(raw_value, &n, 4);
  } else {
    memcpy(raw_value, &temperature, sizeof(double));
  }
}

void supla_channel_temphum_value::set_humidity(double humidity) {
  if (is_humidity_available()) {
    if (humidity < incorrect_humidity()) {
      humidity = incorrect_humidity();
    }

    int n = humidity * 1000;
    memcpy(&raw_value[4], &n, 4);
  }
}

bool supla_channel_temphum_value::is_differ(supla_abstract_channel_value *value,
                                            bool *significant_change) {
  bool result =
      supla_abstract_channel_value::is_differ(value, significant_change);
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

nlohmann::json supla_channel_temphum_value::get_template_data(void) {
  nlohmann::json result = supla_abstract_channel_value::get_template_data();

  if (func == SUPLA_CHANNELFNC_THERMOMETER ||
      func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE) {
    result["temperature"] = get_temperature();
  }

  if (func == SUPLA_CHANNELFNC_HUMIDITY ||
      func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE) {
    result["humidity"] = get_humidity();
  }

  return result;
}

bool supla_channel_temphum_value::get_vbt_value(_vbt_var_name_e var_name,
                                                double *value) {
  switch (var_name) {
    case var_name_temperature: {
      double temperature = get_temperature();
      if (temperature > supla_channel_temphum_value::incorrect_temperature()) {
        *value = temperature;
        return true;
      }
    } break;
    case var_name_humidity:
      if (is_humidity_available()) {
        double humidity = get_humidity();
        if (humidity > supla_channel_temphum_value::incorrect_humidity()) {
          *value = get_humidity();
          return true;
        }
      }
      break;
    default:
      break;
  }

  return false;
}
