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

#include "hvac_config.h"

using std::map;
using std::string;

#define FIELD_MAIN_THERMOMETER_CHANNEL_NO 1
#define FIELD_AUX_THERMOMETER_CHANNEL_NO 2
#define FIELD_AUX_THERMOMETER_TYPE 3
#define FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED 4
#define FIELD_AVAILABLE_ALGORITHMS 5
#define FIELD_USED_ALGORITHM 6
#define FIELD_MIN_ON_TIME_S 7
#define FIELD_MIN_OFF_TIME_S 8
#define FIELD_OUTPUT_VALUE_ON_ERROR 9
#define FIELD_TEMPERATURES 10

const map<unsigned _supla_int16_t, string> hvac_config::field_map = {
    {FIELD_MAIN_THERMOMETER_CHANNEL_NO, "mainThermometerChannelNo"},
    {FIELD_AUX_THERMOMETER_CHANNEL_NO, "auxThermometerChannelNo"},
    {FIELD_AUX_THERMOMETER_TYPE, "auxThermometerType"},
    {FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED,
     "antiFreezeAndOverheatProtectionEnabled"},
    {FIELD_AVAILABLE_ALGORITHMS, "availableAlgorithms"},
    {FIELD_USED_ALGORITHM, "usedAlgorithm"},
    {FIELD_MIN_ON_TIME_S, "minOnTimeS"},
    {FIELD_MIN_OFF_TIME_S, "minOffTimeS"},
    {FIELD_OUTPUT_VALUE_ON_ERROR, "outputValueOnError"},
    {FIELD_TEMPERATURES, "temperatures"}};

hvac_config::hvac_config(void) : channel_json_config() {}

hvac_config::hvac_config(supla_json_config *root) : channel_json_config(root) {}

cJSON *hvac_config::get_hvac_root(bool force) {
  const char hvac[] = "hvac";

  cJSON *root = get_user_root();
  if (root) {
    cJSON *hvac_root = cJSON_GetObjectItem(root, hvac);
    if (!hvac_root) {
      hvac_root = cJSON_AddObjectToObject(root, hvac);
    }
    return hvac_root;
  }
  return nullptr;
}

cJSON *hvac_config::get_hvac_root(void) { return get_hvac_root(true); }

string hvac_config::aux_thermometer_type_to_string(unsigned char type) {
  switch (type) {
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_NOT_SET:
      return "NotSet";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_DISABLED:
      return "Disabled";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR:
      return "Floor";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_WATER:
      return "Wather";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER:
      return "GenericHeater";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER:
      return "GenericCooler";
  }

  return "";
}

unsigned char hvac_config::string_to_aux_thermometer_type(const string &type) {
  if (type == "Disabled") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_DISABLED;
  } else if (type == "Floor") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR;
  } else if (type == "Wather") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_WATER;
  } else if (type == "GenericHeater") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER;
  } else if (type == "GenericCooler") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER;
  }

  return SUPLA_HVAC_AUX_THERMOMETER_TYPE_NOT_SET;
}

string hvac_config::alg_to_string(unsigned _supla_int16_t alg) {
  switch (alg) {
    case SUPLA_HVAC_ALGORITHM_ON_OFF:
      return "OnOff";
  }
  return "";
}

unsigned _supla_int16_t hvac_config::string_to_alg(const string &alg) {
  if (alg == "OnOff") {
    return SUPLA_HVAC_ALGORITHM_ON_OFF;
  }

  return 0;
}

void hvac_config::merge(supla_json_config *_dst) {
  hvac_config dst(_dst);
  supla_json_config::merge(get_hvac_root(), dst.get_hvac_root(), field_map,
                           false);
}

void hvac_config::add_algorithm_to_array(cJSON *root, cJSON *algs,
                                         TChannelConfig_HVAC *config,
                                         unsigned _supla_int16_t alg) {
  if (root && algs) {
    if (config->AvailableAlgorithms & alg) {
      cJSON *alg_json = cJSON_CreateString(alg_to_string(alg).c_str());
      if (alg_json) {
        cJSON_AddItemToArray(algs, alg_json);
      }
    }
  }
}

void hvac_config::set_config(TChannelConfig_HVAC *config) {
  if (!config) {
    return;
  }

  cJSON *root = get_hvac_root();
  if (!root) {
    return;
  }

  set_item_value(root, field_map.at(FIELD_MAIN_THERMOMETER_CHANNEL_NO).c_str(),
                 cJSON_Number, true, nullptr, config->MainThermometerChannelNo);

  set_item_value(root, field_map.at(FIELD_AUX_THERMOMETER_CHANNEL_NO).c_str(),
                 cJSON_Number, true, nullptr, config->AuxThermometerChannelNo);

  set_item_value(
      root, field_map.at(FIELD_AUX_THERMOMETER_TYPE).c_str(), cJSON_String,
      true, aux_thermometer_type_to_string(config->AuxThermometerType).c_str(),
      0);

  set_item_value(
      root,
      field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED).c_str(),
      config->AntiFreezeAndOverheatProtectionEnabled ? cJSON_True : cJSON_False,
      true, nullptr, 0);

  cJSON *algs = cJSON_GetObjectItem(
      root, field_map.at(FIELD_AVAILABLE_ALGORITHMS).c_str());

  if (algs) {
    cJSON_DetachItemViaPointer(root, algs);
    cJSON_Delete(algs);
  }

  algs = cJSON_AddArrayToObject(
      root, field_map.at(FIELD_AVAILABLE_ALGORITHMS).c_str());

  add_algorithm_to_array(root, algs, config, SUPLA_HVAC_ALGORITHM_ON_OFF);

  set_item_value(root, field_map.at(FIELD_USED_ALGORITHM).c_str(), cJSON_String,
                 true, alg_to_string(config->UsedAlgorithm).c_str(), 0);

  if (config->MinOffTimeS >= 0 && config->MinOffTimeS <= 600) {
    set_item_value(root, field_map.at(FIELD_MIN_OFF_TIME_S).c_str(),
                   cJSON_Number, true, nullptr, config->MinOffTimeS);
  }

  if (config->MinOnTimeS >= 0 && config->MinOnTimeS <= 600) {
    set_item_value(root, field_map.at(FIELD_MIN_ON_TIME_S).c_str(),
                   cJSON_Number, true, nullptr, config->MinOnTimeS);
  }

  if (config->OutputValueOnError >= -100 && config->OutputValueOnError <= 100) {
    set_item_value(root, field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR).c_str(),
                   cJSON_Number, true, nullptr, config->OutputValueOnError);
  }

  cJSON *temperatures =
      cJSON_GetObjectItem(root, field_map.at(FIELD_TEMPERATURES).c_str());

  if (temperatures) {
    cJSON_DetachItemViaPointer(root, temperatures);
    cJSON_Delete(temperatures);
  }

  temperatures =
      cJSON_AddObjectToObject(root, field_map.at(FIELD_TEMPERATURES).c_str());

  unsigned _supla_int_t idx = 1;
  int size = sizeof(config->Temperatures.Temperature) / sizeof(_supla_int16_t);

  for (int a = 0; a < size; a++) {
    if (config->Temperatures.Index & idx) {
      string name = std::to_string(a + 1);
      cJSON_AddNumberToObject(temperatures, name.c_str(),
                              config->Temperatures.Temperature[a]);
    }
    idx <<= 1;
  }
}

bool hvac_config::get_config(TChannelConfig_HVAC *config) {
  if (!config) {
    return false;
  }

  *config = {};

  cJSON *root = get_hvac_root(false);
  if (!root) {
    return false;
  }

  bool result = false;

  double dbl_value = 0;
  if (get_double(root, field_map.at(FIELD_MAIN_THERMOMETER_CHANNEL_NO).c_str(),
                 &dbl_value)) {
    config->MainThermometerChannelNo = dbl_value;
    result = true;
  }

  if (get_double(root, field_map.at(FIELD_AUX_THERMOMETER_CHANNEL_NO).c_str(),
                 &dbl_value)) {
    config->AuxThermometerChannelNo = dbl_value;
    result = true;
  }

  std::string str_value;
  if (get_string(root, field_map.at(FIELD_AUX_THERMOMETER_TYPE).c_str(),
                 &str_value)) {
    config->AuxThermometerType = string_to_aux_thermometer_type(str_value);
    result = true;
  }

  bool bool_value;
  if (get_bool(root,
               field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED)
                   .c_str(),
               &bool_value)) {
    config->AntiFreezeAndOverheatProtectionEnabled = bool_value ? 1 : 0;
    result = true;
  }

  cJSON *algs = cJSON_GetObjectItem(
      root, field_map.at(FIELD_AVAILABLE_ALGORITHMS).c_str());

  if (algs && cJSON_IsArray(algs)) {
    for (int a = 0; a < cJSON_GetArraySize(algs); a++) {
      cJSON *item = cJSON_GetArrayItem(algs, a);
      if (item && cJSON_IsString(item)) {
        config->AvailableAlgorithms |=
            string_to_alg(cJSON_GetStringValue(item));
        result = true;
      }
    }
  }

  if (get_string(root, field_map.at(FIELD_USED_ALGORITHM).c_str(),
                 &str_value)) {
    config->UsedAlgorithm = string_to_alg(str_value);
    result = true;
  }

  if (get_double(root, field_map.at(FIELD_MIN_OFF_TIME_S).c_str(),
                 &dbl_value) &&
      dbl_value >= 0 && dbl_value <= 600) {
    config->MinOffTimeS = dbl_value;
    result = true;
  }

  if (get_double(root, field_map.at(FIELD_MIN_ON_TIME_S).c_str(), &dbl_value) &&
      dbl_value >= 0 && dbl_value <= 600) {
    config->MinOnTimeS = dbl_value;
    result = true;
  }

  if (get_double(root, field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR).c_str(),
                 &dbl_value) &&
      dbl_value >= -100 && dbl_value <= 100) {
    config->OutputValueOnError = dbl_value;
    result = true;
  }

  cJSON *temperatures =
      cJSON_GetObjectItem(root, field_map.at(FIELD_TEMPERATURES).c_str());

  if (temperatures && cJSON_IsObject(temperatures)) {
    int size =
        sizeof(config->Temperatures.Temperature) / sizeof(_supla_int16_t);
    for (int a = 0; a < size; a++) {
      string name = std::to_string(a + 1);
      cJSON *item = cJSON_GetObjectItem(temperatures, name.c_str());
      if (item && cJSON_IsNumber(item)) {
        config->Temperatures.Temperature[a] = cJSON_GetNumberValue(item);
        config->Temperatures.Index |= 1 << a;
        result = true;
      }
    }
  }

  return result;
}
