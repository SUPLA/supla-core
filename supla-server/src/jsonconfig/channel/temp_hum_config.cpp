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

#include "temp_hum_config.h"

using std::map;
using std::string;

#define FIELD_TEMPERATURE_ADJUSTMENT 1
#define FIELD_HUMIDITY_ADJUSTMENT 2
#define FIELD_ADJUSTMENT_APPLIED_BY_DEVICE 3
#define FIELD_MIN_TEMPERATURE_ADJUSTMENT 4
#define FIELD_MAX_TEMPERATURE_ADJUSTMENT 5
#define FIELD_MIN_HUMIDITY_ADJUSTMENT 6
#define FIELD_MAX_HUMIDITY_ADJUSTMENT 7

const map<unsigned _supla_int16_t, string> temp_hum_config::field_map = {
    {FIELD_TEMPERATURE_ADJUSTMENT, "temperatureAdjustment"},
    {FIELD_HUMIDITY_ADJUSTMENT, "humidityAdjustment"},
    {FIELD_ADJUSTMENT_APPLIED_BY_DEVICE, "adjustmentAppliedByDevice"},
    {FIELD_MIN_TEMPERATURE_ADJUSTMENT, "minTemperatureAdjustment"},
    {FIELD_MAX_TEMPERATURE_ADJUSTMENT, "maxTemperatureAdjustment"},
    {FIELD_MIN_HUMIDITY_ADJUSTMENT, "minHumidityAdjustment"},
    {FIELD_MAX_HUMIDITY_ADJUSTMENT, "maxHumidityAdjustment"}};

temp_hum_config::temp_hum_config(void) : supla_json_config() {}

temp_hum_config::temp_hum_config(supla_json_config *root)
    : supla_json_config(root) {}

void temp_hum_config::merge(supla_json_config *_dst) {
  temp_hum_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           false);

  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           field_map, true);
}

void temp_hum_config::set_adjustment(cJSON *root, int field_id,
                                     _supla_int16_t value) {
  if (value) {
    set_item_value(root, field_map.at(field_id).c_str(), cJSON_Number, true,
                   nullptr, nullptr, value);
  } else {
    cJSON_DeleteItemFromObject(root, field_map.at(field_id).c_str());
  }
}

void temp_hum_config::set_config(
    TChannelConfig_TemperatureAndHumidity *config) {
  if (!config) {
    return;
  }

  cJSON *root = get_user_root();
  if (!root) {
    return;
  }

  set_item_value(root, field_map.at(FIELD_TEMPERATURE_ADJUSTMENT).c_str(),
                 cJSON_Number, true, nullptr, nullptr,
                 config->TemperatureAdjustment);

  set_item_value(root, field_map.at(FIELD_HUMIDITY_ADJUSTMENT).c_str(),
                 cJSON_Number, true, nullptr, nullptr,
                 config->HumidityAdjustment);

  set_item_value(root, field_map.at(FIELD_ADJUSTMENT_APPLIED_BY_DEVICE).c_str(),
                 config->AdjustmentAppliedByDevice ? cJSON_True : cJSON_False,
                 true, nullptr, nullptr, 0);

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return;
  }

  set_adjustment(properties_root, FIELD_MIN_TEMPERATURE_ADJUSTMENT,
                 config->MinTemperatureAdjustment);

  set_adjustment(properties_root, FIELD_MAX_TEMPERATURE_ADJUSTMENT,
                 config->MaxTemperatureAdjustment);

  set_adjustment(properties_root, FIELD_MIN_HUMIDITY_ADJUSTMENT,
                 config->MinHumidityAdjustment);

  set_adjustment(properties_root, FIELD_MAX_HUMIDITY_ADJUSTMENT,
                 config->MaxHumidityAdjustment);
}

bool temp_hum_config::get_config(
    TChannelConfig_TemperatureAndHumidity *config) {
  if (!config) {
    return false;
  }

  *config = {};

  cJSON *root = get_user_root();
  if (!root) {
    return false;
  }

  bool result = false;

  double dbl_value = 0;
  if (get_double(root, field_map.at(FIELD_TEMPERATURE_ADJUSTMENT).c_str(),
                 &dbl_value)) {
    config->TemperatureAdjustment = dbl_value;
    result = true;
  }

  if (get_double(root, field_map.at(FIELD_HUMIDITY_ADJUSTMENT).c_str(),
                 &dbl_value)) {
    config->HumidityAdjustment = dbl_value;
    result = true;
  }

  bool bool_value;
  if (get_bool(root, field_map.at(FIELD_ADJUSTMENT_APPLIED_BY_DEVICE).c_str(),
               &bool_value)) {
    config->AdjustmentAppliedByDevice = bool_value ? 1 : 0;
    result = true;
  }

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return result;
  }

  if (get_double(properties_root,
                 field_map.at(FIELD_MIN_TEMPERATURE_ADJUSTMENT).c_str(),
                 &dbl_value)) {
    config->MinTemperatureAdjustment = dbl_value;
    result = true;
  }

  if (get_double(properties_root,
                 field_map.at(FIELD_MAX_TEMPERATURE_ADJUSTMENT).c_str(),
                 &dbl_value)) {
    config->MaxTemperatureAdjustment = dbl_value;
    result = true;
  }

  if (get_double(properties_root,
                 field_map.at(FIELD_MIN_HUMIDITY_ADJUSTMENT).c_str(),
                 &dbl_value)) {
    config->MinHumidityAdjustment = dbl_value;
    result = true;
  }

  if (get_double(properties_root,
                 field_map.at(FIELD_MAX_HUMIDITY_ADJUSTMENT).c_str(),
                 &dbl_value)) {
    config->MaxHumidityAdjustment = dbl_value;
    result = true;
  }

  return result;
}

_supla_int16_t temp_hum_config::get_temperature_adjustment(void) {
  TChannelConfig_TemperatureAndHumidity config = {};
  if (get_config(&config)) {
    return config.TemperatureAdjustment;
  }

  return 0;
}

_supla_int16_t temp_hum_config::get_humidity_adjustment(void) {
  TChannelConfig_TemperatureAndHumidity config = {};
  if (get_config(&config)) {
    return config.HumidityAdjustment;
  }

  return 0;
}

double temp_hum_config::get_temperature_adjustment_dbl(void) {
  return get_temperature_adjustment() * 0.01;
}

double temp_hum_config::get_humidity_adjustment_dbl(void) {
  return get_humidity_adjustment() * 0.01;
}

bool temp_hum_config::is_adjustment_applied_by_device(void) {
  TChannelConfig_TemperatureAndHumidity config = {};
  if (get_config(&config)) {
    return config.AdjustmentAppliedByDevice;
  }

  return false;
}
