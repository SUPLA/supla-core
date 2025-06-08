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

#include "channel_hvac_value_with_temphum.h"

#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "device/value/channel_hp_thermostat_value.h"
#include "device/value/channel_hvac_value.h"
#include "device/value/channel_temphum_value.h"
#include "jsonconfig/channel/hvac_config.h"

supla_channel_hvac_value_with_temphum::supla_channel_hvac_value_with_temphum(
    void)
    : supla_channel_hvac_value() {
  temperature = supla_channel_temphum_value::incorrect_temperature() * 100.0;
  humidity = supla_channel_temphum_value::incorrect_humidity() * 100.0;
}

supla_channel_hvac_value_with_temphum::supla_channel_hvac_value_with_temphum(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_hvac_value(raw_value) {
  temperature = supla_channel_temphum_value::incorrect_temperature() * 100.0;
  humidity = supla_channel_temphum_value::incorrect_humidity() * 100.0;
}

supla_channel_hvac_value_with_temphum::supla_channel_hvac_value_with_temphum(
    supla_channel_hvac_value *value)
    : supla_channel_hvac_value() {
  if (value) {
    value->get_raw_value(raw_value);
  }
  temperature = supla_channel_temphum_value::incorrect_temperature() * 100.0;
  humidity = supla_channel_temphum_value::incorrect_humidity() * 100.0;
}

supla_abstract_channel_value *
supla_channel_hvac_value_with_temphum::copy(  // NOLINT
    void) const {                             // NOLINT
  supla_channel_hvac_value_with_temphum *result =
      new supla_channel_hvac_value_with_temphum(raw_value);
  result->temperature = this->temperature;
  result->humidity = this->humidity;

  return result;
}

void supla_channel_hvac_value_with_temphum::set_temperature(short temperature) {
  this->temperature = temperature;
}

short supla_channel_hvac_value_with_temphum::get_temperature(void) {
  return temperature;
}

double supla_channel_hvac_value_with_temphum::get_temperature_dbl(void) {
  return temperature / 100.00;
}

void supla_channel_hvac_value_with_temphum::set_humidity(short humidity) {
  this->humidity = humidity;
}

short supla_channel_hvac_value_with_temphum::get_humidity(void) {
  return humidity;
}

double supla_channel_hvac_value_with_temphum::get_humidity_dbl(void) {
  return humidity / 100.00;
}

// static
void supla_channel_hvac_value_with_temphum::expand(
    supla_abstract_channel_value **value, supla_channel_fragment *fragment,
    supla_abstract_channel_property_getter *getter) {
  if (!value || !*value || !getter || !getter->get_user_id() || !fragment) {
    return;
  }

  supla_channel_hvac_value *hvac_value =
      dynamic_cast<supla_channel_hvac_value *>(*value);
  if (hvac_value) {
    supla_channel_hvac_value_with_temphum *result =
        new supla_channel_hvac_value_with_temphum(hvac_value);

    supla_json_config *json_config = getter->get_detached_json_config(
        getter->get_user_id(), fragment->get_device_id(),
        fragment->get_channel_id());

    if (json_config) {
      hvac_config json_hvac(json_config);
      TChannelConfig_HVAC native_config = {};
      json_hvac.get_config(&native_config, fragment->get_channel_number());
      delete json_config;

      if (native_config.MainThermometerChannelNo !=
          fragment->get_channel_number()) {
        int main_thermometer_channel_id = getter->get_channel_id(
            getter->get_user_id(), fragment->get_device_id(),
            native_config.MainThermometerChannelNo);

        if (main_thermometer_channel_id) {
          supla_abstract_channel_value *th_value = getter->get_value(
              getter->get_user_id(), fragment->get_device_id(),
              main_thermometer_channel_id);
          if (th_value) {
            supla_channel_temphum_value *temphum =
                dynamic_cast<supla_channel_temphum_value *>(th_value);
            if (temphum) {
              result->set_temperature(temphum->get_temperature() * 100.0);
              if (temphum->is_humidity_available()) {
                result->set_humidity(temphum->get_humidity() * 100.0);
              }
            }
            delete th_value;
          }
        }
      }
    }

    delete *value;
    *value = result;
    return;
  }

  supla_channel_hp_thermostat_value *hp_value =
      dynamic_cast<supla_channel_hp_thermostat_value *>(*value);

  if (hp_value) {
    supla_channel_hvac_value_with_temphum *result =
        new supla_channel_hvac_value_with_temphum();
    result->set_setpoint_temperature_heat(hp_value->get_preset_temperature() *
                                          100);
    result->set_temperature(hp_value->get_measured_temperature() * 100);

    supla_abstract_channel_extended_value *ext_val = getter->get_extended_value(
        getter->get_user_id(), fragment->get_device_id(),
        fragment->get_channel_id());

    if (ext_val) {
      supla_channel_thermostat_extended_value *th_ext_val =
          dynamic_cast<supla_channel_thermostat_extended_value *>(ext_val);
      if (th_ext_val) {
        supla_channel_hp_thermostat_ev_decorator decorator(th_ext_val);

        result->set_on(decorator.is_heating() ? 1 : 0);
        result->set_mode(decorator.get_hvac_mode());
        result->set_flags(result->get_flags() | decorator.get_hvac_flags());
      }
      delete ext_val;
    }

    delete *value;
    *value = result;
    return;
  }
}
