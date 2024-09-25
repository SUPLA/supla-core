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
#define FIELD_BINARY_SENSOR_CHANNEL_NO 4
#define FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED 5
#define FIELD_AVAILABLE_ALGORITHMS 6
#define FIELD_USED_ALGORITHM 7
#define FIELD_MIN_ON_TIME_S 8
#define FIELD_MIN_OFF_TIME_S 9
#define FIELD_OUTPUT_VALUE_ON_ERROR 10
#define FIELD_SUBSUNCTION 11
#define FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE 12
#define FIELD_AUX_MIN_MAX_SETPOINT_ENABLED 13
#define FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS 14
#define FIELD_TEMPERATURES 15

#define FIELD_MASTER_THERMOSTAT_CHANNEL_NO 16
#define FIELD_HEAT_OR_COLD_SOURCE_SWITCH 17
#define FIELD_PUMP_SWITCH 18

#define FIELD_HIDDEN_CONFIG_FIELDS 19
#define FIELD_READONLY_CONFIG_FIELDS 20
#define FIELD_HIDDEN_TEMPERATURE_CONFIG_FIELDS 21
#define FIELD_READONLY_TEMPERATURE_CONFIG_FIELDS 22

const map<unsigned _supla_int16_t, string> hvac_config::field_map = {
    {FIELD_MAIN_THERMOMETER_CHANNEL_NO, "mainThermometerChannelNo"},
    {FIELD_AUX_THERMOMETER_CHANNEL_NO, "auxThermometerChannelNo"},
    {FIELD_AUX_THERMOMETER_TYPE, "auxThermometerType"},
    {FIELD_AUX_MIN_MAX_SETPOINT_ENABLED, "auxMinMaxSetpointEnabled"},
    {FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS, "useSeparateHeatCoolOutputs"},
    {FIELD_BINARY_SENSOR_CHANNEL_NO, "binarySensorChannelNo"},
    {FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED,
     "antiFreezeAndOverheatProtectionEnabled"},
    {FIELD_AVAILABLE_ALGORITHMS, "availableAlgorithms"},
    {FIELD_USED_ALGORITHM, "usedAlgorithm"},
    {FIELD_MIN_ON_TIME_S, "minOnTimeS"},
    {FIELD_MIN_OFF_TIME_S, "minOffTimeS"},
    {FIELD_OUTPUT_VALUE_ON_ERROR, "outputValueOnError"},
    {FIELD_SUBSUNCTION, "subfunction"},
    {FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE,
     "temperatureSetpointChangeSwitchesToManualMode"},
    {FIELD_TEMPERATURES, "temperatures"},
    {FIELD_MASTER_THERMOSTAT_CHANNEL_NO, "masterThermostatChannelNo"},
    {FIELD_HEAT_OR_COLD_SOURCE_SWITCH, "heatOrColdSourceSwitchChannelNo"},
    {FIELD_PUMP_SWITCH, "pumpSwitchChannelNo"},
    {FIELD_HIDDEN_CONFIG_FIELDS, "hiddenConfigFields"},
    {FIELD_READONLY_CONFIG_FIELDS, "readOnlyConfigFields"},
    {FIELD_HIDDEN_TEMPERATURE_CONFIG_FIELDS, "hiddenTemperatureConfigFields"},
    {FIELD_READONLY_TEMPERATURE_CONFIG_FIELDS,
     "readOnlyTemperatureConfigFields"}};

const map<unsigned int, string> hvac_config::temperatures_map = {
    {TEMPERATURE_FREEZE_PROTECTION, "freezeProtection"},
    {TEMPERATURE_ECO, "eco"},
    {TEMPERATURE_COMFORT, "comfort"},
    {TEMPERATURE_BOOST, "boost"},
    {TEMPERATURE_HEAT_PROTECTION, "heatProtection"},
    {TEMPERATURE_HISTERESIS, "histeresis"},
    {TEMPERATURE_BELOW_ALARM, "belowAlarm"},
    {TEMPERATURE_ABOVE_ALARM, "aboveAlarm"},
    {TEMPERATURE_AUX_MIN_SETPOINT, "auxMinSetpoint"},
    {TEMPERATURE_AUX_MAX_SETPOINT, "auxMaxSetpoint"},
    {TEMPERATURE_ROOM_MIN, "roomMin"},
    {TEMPERATURE_ROOM_MAX, "roomMax"},
    {TEMPERATURE_AUX_MIN, "auxMin"},
    {TEMPERATURE_AUX_MAX, "auxMax"},
    {TEMPERATURE_HISTERESIS_MIN, "histeresisMin"},
    {TEMPERATURE_HISTERESIS_MAX, "histeresisMax"},
    {TEMPERATURE_HEAT_COOL_OFFSET_MIN, "heatCoolOffsetMin"},
    {TEMPERATURE_HEAT_COOL_OFFSET_MAX, "heatCoolOffsetMax"}};

const unsigned int hvac_config::readonly_temperatures =
    TEMPERATURE_ROOM_MIN | TEMPERATURE_ROOM_MAX | TEMPERATURE_AUX_MIN |
    TEMPERATURE_AUX_MAX | TEMPERATURE_HISTERESIS_MIN |
    TEMPERATURE_HISTERESIS_MAX | TEMPERATURE_HEAT_COOL_OFFSET_MIN |
    TEMPERATURE_HEAT_COOL_OFFSET_MAX;

hvac_config::hvac_config(void) : supla_json_config() {}

hvac_config::hvac_config(supla_json_config *root) : supla_json_config(root) {}

string hvac_config::aux_thermometer_type_to_string(unsigned char type) {
  switch (type) {
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_NOT_SET:
      return "NOT_SET";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_DISABLED:
      return "DISABLED";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR:
      return "FLOOR";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_WATER:
      return "WATER";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER:
      return "GENERIC_HEATER";
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER:
      return "GENERIC_COOLER";
  }

  return "";
}

unsigned char hvac_config::string_to_aux_thermometer_type(const string &type) {
  if (type == "DISABLED") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_DISABLED;
  } else if (type == "FLOOR") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR;
  } else if (type == "WATER") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_WATER;
  } else if (type == "GENERIC_HEATER") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER;
  } else if (type == "GENERIC_COOLER") {
    return SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER;
  }

  return SUPLA_HVAC_AUX_THERMOMETER_TYPE_NOT_SET;
}

string hvac_config::alg_to_string(unsigned _supla_int16_t alg) {
  switch (alg) {
    case SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_MIDDLE:
      return "ON_OFF_SETPOINT_MIDDLE";
    case SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_AT_MOST:
      return "ON_OFF_SETPOINT_AT_MOST";
    case SUPLA_HVAC_ALGORITHM_PID:
      return "PID";
  }
  return "";
}

unsigned _supla_int16_t hvac_config::string_to_alg(const string &alg) {
  if (alg == "ON_OFF_SETPOINT_MIDDLE") {
    return SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_MIDDLE;
  } else if (alg == "ON_OFF_SETPOINT_AT_MOST") {
    return SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_AT_MOST;
  } else if (alg == "PID") {
    return SUPLA_HVAC_ALGORITHM_PID;
  }

  return 0;
}

string hvac_config::subfunction_to_string(unsigned char subfunction) {
  switch (subfunction) {
    case SUPLA_HVAC_SUBFUNCTION_HEAT:
      return "HEAT";
    case SUPLA_HVAC_SUBFUNCTION_COOL:
      return "COOL";
  }
  return "NOT_SET";
}

unsigned char hvac_config::string_to_subfunction(const string &subfunction) {
  if (subfunction == "HEAT") {
    return SUPLA_HVAC_SUBFUNCTION_HEAT;
  } else if (subfunction == "COOL") {
    return SUPLA_HVAC_SUBFUNCTION_COOL;
  }

  return SUPLA_HVAC_SUBFUNCTION_NOT_SET;
}

std::string hvac_config::temperature_key_to_string(
    unsigned int temperature_key) {
  for (auto it = temperatures_map.cbegin(); it != temperatures_map.cend();
       ++it) {
    if (it->first == temperature_key) {
      return it->second;
    }
  }
  return "";
}

void hvac_config::merge(supla_json_config *_dst) {
  hvac_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           true);
  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           field_map, true);
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

void hvac_config::set_channel_number(cJSON *root, int field,
                                     unsigned char cfg_channel_number,
                                     unsigned char channel_number) {
  set_item_value(
      root, field_map.at(field).c_str(),
      channel_number == cfg_channel_number ? cJSON_NULL : cJSON_Number, true,
      nullptr, nullptr, cfg_channel_number);
}

bool hvac_config::get_channel_number(cJSON *root, int field,
                                     unsigned char channel_number,
                                     unsigned char *result, bool *is_null) {
  if (is_null) {
    *is_null = false;
  }

  double dbl_value = 0;
  if (get_double(root, field_map.at(field).c_str(), &dbl_value)) {
    *result = dbl_value;
    return true;
  }

  *result = channel_number;
  cJSON *item = cJSON_GetObjectItem(root, field_map.at(field).c_str());
  if (item != nullptr && cJSON_IsNull(item)) {
    if (is_null) {
      *is_null = true;
    }
    return true;
  }

  return false;
}

void hvac_config::set_temperatures(TChannelConfig_HVAC *config, cJSON *root,
                                   unsigned int filter) {
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
    if ((filter & idx) && (config->Temperatures.Index & idx)) {
      string key = temperature_key_to_string(idx);
      if (!key.empty()) {
        cJSON_AddNumberToObject(temperatures, key.c_str(),
                                config->Temperatures.Temperature[a]);
      }
    }
    idx <<= 1;
  }
}

bool hvac_config::get_temperatures(TChannelConfig_HVAC *config, cJSON *root,
                                   unsigned int filter) {
  bool result = false;

  cJSON *temperatures =
      cJSON_GetObjectItem(root, field_map.at(FIELD_TEMPERATURES).c_str());

  if (temperatures && cJSON_IsObject(temperatures)) {
    for (auto it = temperatures_map.cbegin(); it != temperatures_map.cend();
         ++it) {
      if (filter & it->first) {
        cJSON *item = cJSON_GetObjectItem(temperatures, it->second.c_str());
        if (item && cJSON_IsNumber(item)) {
          unsigned int n = 1;
          for (size_t a = 0; a < sizeof(n) * 8; a++) {
            if (n == it->first) {
              config->Temperatures.Temperature[a] = cJSON_GetNumberValue(item);
              config->Temperatures.Index |= n;
              result = true;
              break;
            }
            n <<= 1;
          }
        }
      }
    }
  }

  return result;
}

void hvac_config::set_config(TChannelConfig_HVAC *config,
                             unsigned char channel_number) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return;
  }

  set_channel_number(user_root, FIELD_MAIN_THERMOMETER_CHANNEL_NO,
                     config->MainThermometerChannelNo, channel_number);

  set_channel_number(user_root, FIELD_AUX_THERMOMETER_CHANNEL_NO,
                     config->AuxThermometerChannelNo, channel_number);

  set_item_value(
      user_root, field_map.at(FIELD_AUX_THERMOMETER_TYPE).c_str(), cJSON_String,
      true, nullptr,
      aux_thermometer_type_to_string(config->AuxThermometerType).c_str(), 0);

  set_item_value(user_root,
                 field_map.at(FIELD_AUX_MIN_MAX_SETPOINT_ENABLED).c_str(),
                 config->AuxMinMaxSetpointEnabled ? cJSON_True : cJSON_False,
                 true, nullptr, nullptr, 0);

  set_item_value(user_root,
                 field_map.at(FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS).c_str(),
                 config->UseSeparateHeatCoolOutputs ? cJSON_True : cJSON_False,
                 true, nullptr, nullptr, 0);

  set_channel_number(user_root, FIELD_BINARY_SENSOR_CHANNEL_NO,
                     config->BinarySensorChannelNo, channel_number);

  set_item_value(
      user_root,
      field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED).c_str(),
      config->AntiFreezeAndOverheatProtectionEnabled ? cJSON_True : cJSON_False,
      true, nullptr, nullptr, 0);

  cJSON *algs = cJSON_GetObjectItem(
      properties_root, field_map.at(FIELD_AVAILABLE_ALGORITHMS).c_str());

  if (algs) {
    cJSON_DetachItemViaPointer(properties_root, algs);
    cJSON_Delete(algs);
  }

  algs = cJSON_AddArrayToObject(
      properties_root, field_map.at(FIELD_AVAILABLE_ALGORITHMS).c_str());

  add_algorithm_to_array(user_root, algs, config,
                         SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_MIDDLE);

  add_algorithm_to_array(user_root, algs, config,
                         SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_AT_MOST);

  add_algorithm_to_array(user_root, algs, config, SUPLA_HVAC_ALGORITHM_PID);

  set_item_value(user_root, field_map.at(FIELD_USED_ALGORITHM).c_str(),
                 cJSON_String, true, nullptr,
                 alg_to_string(config->UsedAlgorithm).c_str(), 0);

  if (config->MinOffTimeS >= 0 && config->MinOffTimeS <= 600) {
    set_item_value(user_root, field_map.at(FIELD_MIN_OFF_TIME_S).c_str(),
                   cJSON_Number, true, nullptr, nullptr, config->MinOffTimeS);
  }

  if (config->MinOnTimeS >= 0 && config->MinOnTimeS <= 600) {
    set_item_value(user_root, field_map.at(FIELD_MIN_ON_TIME_S).c_str(),
                   cJSON_Number, true, nullptr, nullptr, config->MinOnTimeS);
  }

  if (config->OutputValueOnError >= -100 && config->OutputValueOnError <= 100) {
    set_item_value(user_root, field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR).c_str(),
                   cJSON_Number, true, nullptr, nullptr,
                   config->OutputValueOnError);
  }

  set_item_value(user_root, field_map.at(FIELD_SUBSUNCTION).c_str(),
                 cJSON_String, true, nullptr,
                 subfunction_to_string(config->Subfunction).c_str(), 0);

  set_item_value(
      user_root,
      field_map.at(FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE)
          .c_str(),
      config->TemperatureSetpointChangeSwitchesToManualMode ? cJSON_True
                                                            : cJSON_False,
      true, nullptr, nullptr, 0);

  set_channel_number(user_root, FIELD_MASTER_THERMOSTAT_CHANNEL_NO,
                     config->MasterThermostatIsSet
                         ? config->MasterThermostatChannelNo
                         : channel_number,
                     channel_number);

  set_channel_number(user_root, FIELD_HEAT_OR_COLD_SOURCE_SWITCH,
                     config->HeatOrColdSourceSwitchIsSet
                         ? config->HeatOrColdSourceSwitchChannelNo
                         : channel_number,
                     channel_number);

  set_channel_number(
      user_root, FIELD_PUMP_SWITCH,
      config->PumpSwitchIsSet ? config->PumpSwitchChannelNo : channel_number,
      channel_number);

  set_temperatures(config, user_root, 0xFFFFFFFF ^ readonly_temperatures);

  set_temperatures(config, properties_root, readonly_temperatures);

  cJSON *readonly = cJSON_CreateArray();
  cJSON *hidden = cJSON_CreateArray();

  if (config->ParameterFlags.MainThermometerChannelNoReadonly) {
    cJSON_AddItemToArray(
        readonly, cJSON_CreateString(
                      field_map.at(FIELD_MAIN_THERMOMETER_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.MainThermometerChannelNoHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(
                    field_map.at(FIELD_MAIN_THERMOMETER_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.AuxThermometerChannelNoReadonly) {
    cJSON_AddItemToArray(
        readonly, cJSON_CreateString(
                      field_map.at(FIELD_AUX_THERMOMETER_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.AuxThermometerChannelNoHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(
                    field_map.at(FIELD_AUX_THERMOMETER_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.BinarySensorChannelNoReadonly) {
    cJSON_AddItemToArray(
        readonly, cJSON_CreateString(
                      field_map.at(FIELD_BINARY_SENSOR_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.BinarySensorChannelNoHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(
                    field_map.at(FIELD_BINARY_SENSOR_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.AuxThermometerTypeReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(field_map.at(FIELD_AUX_THERMOMETER_TYPE).c_str()));
  }

  if (config->ParameterFlags.AuxThermometerTypeHidden) {
    cJSON_AddItemToArray(
        hidden,
        cJSON_CreateString(field_map.at(FIELD_AUX_THERMOMETER_TYPE).c_str()));
  }

  if (config->ParameterFlags.AntiFreezeAndOverheatProtectionEnabledReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(
            field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED)
                .c_str()));
  }

  if (config->ParameterFlags.AntiFreezeAndOverheatProtectionEnabledHidden) {
    cJSON_AddItemToArray(
        hidden,
        cJSON_CreateString(
            field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED)
                .c_str()));
  }

  if (config->ParameterFlags.UsedAlgorithmReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(field_map.at(FIELD_USED_ALGORITHM).c_str()));
  }

  if (config->ParameterFlags.UsedAlgorithmHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(field_map.at(FIELD_USED_ALGORITHM).c_str()));
  }

  if (config->ParameterFlags.MinOnTimeSReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(field_map.at(FIELD_MIN_ON_TIME_S).c_str()));
  }

  if (config->ParameterFlags.MinOnTimeSHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(field_map.at(FIELD_MIN_ON_TIME_S).c_str()));
  }

  if (config->ParameterFlags.MinOffTimeSReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(field_map.at(FIELD_MIN_OFF_TIME_S).c_str()));
  }

  if (config->ParameterFlags.MinOffTimeSHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(field_map.at(FIELD_MIN_OFF_TIME_S).c_str()));
  }

  if (config->ParameterFlags.OutputValueOnErrorReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR).c_str()));
  }

  if (config->ParameterFlags.OutputValueOnErrorHidden) {
    cJSON_AddItemToArray(
        hidden,
        cJSON_CreateString(field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR).c_str()));
  }

  if (config->ParameterFlags.SubfunctionReadonly) {
    cJSON_AddItemToArray(
        readonly, cJSON_CreateString(field_map.at(FIELD_SUBSUNCTION).c_str()));
  }

  if (config->ParameterFlags.SubfunctionHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(field_map.at(FIELD_SUBSUNCTION).c_str()));
  }

  if (config->ParameterFlags.AuxMinMaxSetpointEnabledReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(
            field_map.at(FIELD_AUX_MIN_MAX_SETPOINT_ENABLED).c_str()));
  }

  if (config->ParameterFlags.AuxMinMaxSetpointEnabledHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(
                    field_map.at(FIELD_AUX_MIN_MAX_SETPOINT_ENABLED).c_str()));
  }

  if (config->ParameterFlags.UseSeparateHeatCoolOutputsReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(
            field_map.at(FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS).c_str()));
  }

  if (config->ParameterFlags.UseSeparateHeatCoolOutputsHidden) {
    cJSON_AddItemToArray(
        hidden,
        cJSON_CreateString(
            field_map.at(FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS).c_str()));
  }

  if (config->ParameterFlags.MasterThermostatChannelNoReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(
            field_map.at(FIELD_MASTER_THERMOSTAT_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.MasterThermostatChannelNoHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(
                    field_map.at(FIELD_MASTER_THERMOSTAT_CHANNEL_NO).c_str()));
  }

  if (config->ParameterFlags.HeatOrColdSourceSwitchReadonly) {
    cJSON_AddItemToArray(
        readonly, cJSON_CreateString(
                      field_map.at(FIELD_HEAT_OR_COLD_SOURCE_SWITCH).c_str()));
  }

  if (config->ParameterFlags.HeatOrColdSourceSwitchHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(
                    field_map.at(FIELD_HEAT_OR_COLD_SOURCE_SWITCH).c_str()));
  }

  if (config->ParameterFlags.PumpSwitchReadonly) {
    cJSON_AddItemToArray(
        readonly, cJSON_CreateString(field_map.at(FIELD_PUMP_SWITCH).c_str()));
  }

  if (config->ParameterFlags.PumpSwitchHidden) {
    cJSON_AddItemToArray(
        hidden, cJSON_CreateString(field_map.at(FIELD_PUMP_SWITCH).c_str()));
  }

  if (config->ParameterFlags
          .TemperatureSetpointChangeSwitchesToManualModeReadonly) {
    cJSON_AddItemToArray(
        readonly,
        cJSON_CreateString(
            field_map
                .at(FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE)
                .c_str()));
  }

  if (config->ParameterFlags
          .TemperatureSetpointChangeSwitchesToManualModeHidden) {
    cJSON_AddItemToArray(
        hidden,
        cJSON_CreateString(
            field_map
                .at(FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE)
                .c_str()));
  }

  set_item_value(properties_root,
                 field_map.at(FIELD_READONLY_CONFIG_FIELDS).c_str(),
                 cJSON_Object, true, readonly, nullptr, 0);

  set_item_value(properties_root,
                 field_map.at(FIELD_HIDDEN_CONFIG_FIELDS).c_str(), cJSON_Object,
                 true, hidden, nullptr, 0);

  cJSON *temperature_readonly = cJSON_CreateArray();
  cJSON *temperature_hidden = cJSON_CreateArray();

  if (config->ParameterFlags.TemperaturesFreezeProtectionReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_FREEZE_PROTECTION).c_str()));
  }

  if (config->ParameterFlags.TemperaturesFreezeProtectionHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_FREEZE_PROTECTION).c_str()));
  }

  if (config->ParameterFlags.TemperaturesEcoReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(temperatures_map.at(TEMPERATURE_ECO).c_str()));
  }

  if (config->ParameterFlags.TemperaturesEcoHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(temperatures_map.at(TEMPERATURE_ECO).c_str()));
  }

  if (config->ParameterFlags.TemperaturesComfortReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(temperatures_map.at(TEMPERATURE_COMFORT).c_str()));
  }

  if (config->ParameterFlags.TemperaturesComfortHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(temperatures_map.at(TEMPERATURE_COMFORT).c_str()));
  }

  if (config->ParameterFlags.TemperaturesBoostReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(temperatures_map.at(TEMPERATURE_BOOST).c_str()));
  }

  if (config->ParameterFlags.TemperaturesBoostHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(temperatures_map.at(TEMPERATURE_BOOST).c_str()));
  }

  if (config->ParameterFlags.TemperaturesHeatProtectionReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_HEAT_PROTECTION).c_str()));
  }

  if (config->ParameterFlags.TemperaturesHeatProtectionHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_HEAT_PROTECTION).c_str()));
  }

  if (config->ParameterFlags.TemperaturesHisteresisReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_HISTERESIS).c_str()));
  }

  if (config->ParameterFlags.TemperaturesHisteresisHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_HISTERESIS).c_str()));
  }

  if (config->ParameterFlags.TemperaturesBelowAlarmReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_BELOW_ALARM).c_str()));
  }

  if (config->ParameterFlags.TemperaturesBelowAlarmHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_BELOW_ALARM).c_str()));
  }

  if (config->ParameterFlags.TemperaturesAboveAlarmReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_ABOVE_ALARM).c_str()));
  }

  if (config->ParameterFlags.TemperaturesAboveAlarmHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_ABOVE_ALARM).c_str()));
  }

  if (config->ParameterFlags.TemperaturesAuxMinSetpointReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_AUX_MIN_SETPOINT).c_str()));
  }

  if (config->ParameterFlags.TemperaturesAuxMinSetpointHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_AUX_MIN_SETPOINT).c_str()));
  }

  if (config->ParameterFlags.TemperaturesAuxMaxSetpointReadonly) {
    cJSON_AddItemToArray(
        temperature_readonly,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_AUX_MAX_SETPOINT).c_str()));
  }

  if (config->ParameterFlags.TemperaturesAuxMaxSetpointHidden) {
    cJSON_AddItemToArray(
        temperature_hidden,
        cJSON_CreateString(
            temperatures_map.at(TEMPERATURE_AUX_MAX_SETPOINT).c_str()));
  }

  set_item_value(properties_root,
                 field_map.at(FIELD_READONLY_TEMPERATURE_CONFIG_FIELDS).c_str(),
                 cJSON_Object, true, temperature_readonly, nullptr, 0);

  set_item_value(properties_root,
                 field_map.at(FIELD_HIDDEN_TEMPERATURE_CONFIG_FIELDS).c_str(),
                 cJSON_Object, true, temperature_hidden, nullptr, 0);
}

bool hvac_config::get_config(TChannelConfig_HVAC *config,
                             unsigned char channel_number) {
  if (!config) {
    return false;
  }

  *config = {};

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return false;
  }

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return false;
  }

  bool result = false;

  if (get_channel_number(user_root, FIELD_MAIN_THERMOMETER_CHANNEL_NO,
                         channel_number, &config->MainThermometerChannelNo,
                         nullptr)) {
    result = true;
  }

  if (get_channel_number(user_root, FIELD_AUX_THERMOMETER_CHANNEL_NO,
                         channel_number, &config->AuxThermometerChannelNo,
                         nullptr)) {
    result = true;
  }

  std::string str_value;
  if (get_string(user_root, field_map.at(FIELD_AUX_THERMOMETER_TYPE).c_str(),
                 &str_value)) {
    config->AuxThermometerType = string_to_aux_thermometer_type(str_value);
    result = true;
  }

  bool bool_value;
  if (get_bool(user_root,
               field_map.at(FIELD_AUX_MIN_MAX_SETPOINT_ENABLED).c_str(),
               &bool_value)) {
    config->AuxMinMaxSetpointEnabled = bool_value ? 1 : 0;
    result = true;
  }

  if (get_bool(user_root,
               field_map.at(FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS).c_str(),
               &bool_value)) {
    config->UseSeparateHeatCoolOutputs = bool_value ? 1 : 0;
    result = true;
  }

  if (get_channel_number(user_root, FIELD_BINARY_SENSOR_CHANNEL_NO,
                         channel_number, &config->BinarySensorChannelNo,
                         nullptr)) {
    result = true;
  }

  if (get_bool(user_root,
               field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED)
                   .c_str(),
               &bool_value)) {
    config->AntiFreezeAndOverheatProtectionEnabled = bool_value ? 1 : 0;
    result = true;
  }

  cJSON *algs = cJSON_GetObjectItem(
      properties_root, field_map.at(FIELD_AVAILABLE_ALGORITHMS).c_str());

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

  if (get_string(user_root, field_map.at(FIELD_USED_ALGORITHM).c_str(),
                 &str_value)) {
    config->UsedAlgorithm = string_to_alg(str_value);
    result = true;
  }

  double dbl_value = 0;

  if (get_double(user_root, field_map.at(FIELD_MIN_OFF_TIME_S).c_str(),
                 &dbl_value) &&
      dbl_value >= 0 && dbl_value <= 600) {
    config->MinOffTimeS = dbl_value;
    result = true;
  }

  if (get_double(user_root, field_map.at(FIELD_MIN_ON_TIME_S).c_str(),
                 &dbl_value) &&
      dbl_value >= 0 && dbl_value <= 600) {
    config->MinOnTimeS = dbl_value;
    result = true;
  }

  if (get_double(user_root, field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR).c_str(),
                 &dbl_value) &&
      dbl_value >= -100 && dbl_value <= 100) {
    config->OutputValueOnError = dbl_value;
    result = true;
  }

  if (get_string(user_root, field_map.at(FIELD_SUBSUNCTION).c_str(),
                 &str_value)) {
    config->Subfunction = string_to_subfunction(str_value);
    result = true;
  }

  if (get_bool(
          user_root,
          field_map
              .at(FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE)
              .c_str(),
          &bool_value)) {
    config->TemperatureSetpointChangeSwitchesToManualMode = bool_value ? 1 : 0;
    result = true;
  }

  bool is_null = false;

  if (get_channel_number(user_root, FIELD_MASTER_THERMOSTAT_CHANNEL_NO,
                         channel_number, &config->MasterThermostatChannelNo,
                         &is_null)) {
    if (!is_null) {
      config->MasterThermostatIsSet = 1;
    }

    result = true;
  }

  if (get_channel_number(user_root, FIELD_HEAT_OR_COLD_SOURCE_SWITCH,
                         channel_number,
                         &config->HeatOrColdSourceSwitchChannelNo, &is_null)) {
    if (!is_null) {
      config->HeatOrColdSourceSwitchIsSet = 1;
    }

    result = true;
  }

  if (get_channel_number(user_root, FIELD_PUMP_SWITCH, channel_number,
                         &config->PumpSwitchChannelNo, &is_null)) {
    if (!is_null) {
      config->PumpSwitchIsSet = 1;
    }
    result = true;
  }

  if (get_temperatures(config, user_root, 0xFFFFFFFF ^ readonly_temperatures)) {
    result = true;
  }

  if (get_temperatures(config, properties_root, readonly_temperatures)) {
    result = true;
  }

  cJSON *fields = cJSON_GetObjectItem(
      properties_root, field_map.at(FIELD_READONLY_CONFIG_FIELDS).c_str());

  if (fields && cJSON_IsArray(fields)) {
    result = true;

    for (int a = 0; a < cJSON_GetArraySize(fields); a++) {
      cJSON *item = cJSON_GetArrayItem(fields, a);
      if (item && cJSON_IsString(item)) {
        char *str = cJSON_GetStringValue(item);
        if (!str) {
          continue;
        }

        if (str == field_map.at(FIELD_MAIN_THERMOMETER_CHANNEL_NO)) {
          config->ParameterFlags.MainThermometerChannelNoReadonly = 1;
        }

        if (str == field_map.at(FIELD_AUX_THERMOMETER_CHANNEL_NO)) {
          config->ParameterFlags.AuxThermometerChannelNoReadonly = 1;
        }

        if (str == field_map.at(FIELD_BINARY_SENSOR_CHANNEL_NO)) {
          config->ParameterFlags.BinarySensorChannelNoReadonly = 1;
        }

        if (str == field_map.at(FIELD_AUX_THERMOMETER_TYPE)) {
          config->ParameterFlags.AuxThermometerTypeReadonly = 1;
        }

        if (str ==
            field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED)) {
          config->ParameterFlags
              .AntiFreezeAndOverheatProtectionEnabledReadonly = 1;
        }

        if (str == field_map.at(FIELD_USED_ALGORITHM)) {
          config->ParameterFlags.UsedAlgorithmReadonly = 1;
        }

        if (str == field_map.at(FIELD_MIN_ON_TIME_S)) {
          config->ParameterFlags.MinOnTimeSReadonly = 1;
        }

        if (str == field_map.at(FIELD_MIN_OFF_TIME_S)) {
          config->ParameterFlags.MinOffTimeSReadonly = 1;
        }

        if (str == field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR)) {
          config->ParameterFlags.OutputValueOnErrorReadonly = 1;
        }

        if (str == field_map.at(FIELD_SUBSUNCTION)) {
          config->ParameterFlags.SubfunctionReadonly = 1;
        }

        if (str == field_map.at(FIELD_AUX_MIN_MAX_SETPOINT_ENABLED)) {
          config->ParameterFlags.AuxMinMaxSetpointEnabledReadonly = 1;
        }

        if (str == field_map.at(FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS)) {
          config->ParameterFlags.UseSeparateHeatCoolOutputsReadonly = 1;
        }

        if (str == field_map.at(FIELD_MASTER_THERMOSTAT_CHANNEL_NO)) {
          config->ParameterFlags.MasterThermostatChannelNoReadonly = 1;
        }

        if (str == field_map.at(FIELD_HEAT_OR_COLD_SOURCE_SWITCH)) {
          config->ParameterFlags.HeatOrColdSourceSwitchReadonly = 1;
        }

        if (str == field_map.at(FIELD_PUMP_SWITCH)) {
          config->ParameterFlags.PumpSwitchReadonly = 1;
        }

        if (str ==
            field_map.at(
                FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE)) {
          config->ParameterFlags
              .TemperatureSetpointChangeSwitchesToManualModeReadonly = 1;
        }
      }
    }
  }

  fields = cJSON_GetObjectItem(
      properties_root, field_map.at(FIELD_HIDDEN_CONFIG_FIELDS).c_str());

  if (fields && cJSON_IsArray(fields)) {
    result = true;

    for (int a = 0; a < cJSON_GetArraySize(fields); a++) {
      cJSON *item = cJSON_GetArrayItem(fields, a);
      if (item && cJSON_IsString(item)) {
        char *str = cJSON_GetStringValue(item);
        if (!str) {
          continue;
        }

        if (str == field_map.at(FIELD_MAIN_THERMOMETER_CHANNEL_NO)) {
          config->ParameterFlags.MainThermometerChannelNoHidden = 1;
        }

        if (str == field_map.at(FIELD_AUX_THERMOMETER_CHANNEL_NO)) {
          config->ParameterFlags.AuxThermometerChannelNoHidden = 1;
        }

        if (str == field_map.at(FIELD_BINARY_SENSOR_CHANNEL_NO)) {
          config->ParameterFlags.BinarySensorChannelNoHidden = 1;
        }

        if (str == field_map.at(FIELD_AUX_THERMOMETER_TYPE)) {
          config->ParameterFlags.AuxThermometerTypeHidden = 1;
        }

        if (str ==
            field_map.at(FIELD_ANTI_FREEZE_AND_OVERHEAT_PRETECTION_ENABLED)) {
          config->ParameterFlags.AntiFreezeAndOverheatProtectionEnabledHidden =
              1;
        }

        if (str == field_map.at(FIELD_USED_ALGORITHM)) {
          config->ParameterFlags.UsedAlgorithmHidden = 1;
        }

        if (str == field_map.at(FIELD_MIN_ON_TIME_S)) {
          config->ParameterFlags.MinOnTimeSHidden = 1;
        }

        if (str == field_map.at(FIELD_MIN_OFF_TIME_S)) {
          config->ParameterFlags.MinOffTimeSHidden = 1;
        }

        if (str == field_map.at(FIELD_OUTPUT_VALUE_ON_ERROR)) {
          config->ParameterFlags.OutputValueOnErrorHidden = 1;
        }

        if (str == field_map.at(FIELD_SUBSUNCTION)) {
          config->ParameterFlags.SubfunctionHidden = 1;
        }

        if (str == field_map.at(FIELD_AUX_MIN_MAX_SETPOINT_ENABLED)) {
          config->ParameterFlags.AuxMinMaxSetpointEnabledHidden = 1;
        }

        if (str == field_map.at(FIELD_USE_SEPARATE_HEAT_COOL_OUTPUTS)) {
          config->ParameterFlags.UseSeparateHeatCoolOutputsHidden = 1;
        }

        if (str == field_map.at(FIELD_MASTER_THERMOSTAT_CHANNEL_NO)) {
          config->ParameterFlags.MasterThermostatChannelNoHidden = 1;
        }

        if (str == field_map.at(FIELD_HEAT_OR_COLD_SOURCE_SWITCH)) {
          config->ParameterFlags.HeatOrColdSourceSwitchHidden = 1;
        }

        if (str == field_map.at(FIELD_PUMP_SWITCH)) {
          config->ParameterFlags.PumpSwitchHidden = 1;
        }

        if (str ==
            field_map.at(
                FIELD_TEMPERATURE_SETPOINT_CHANGE_SWITCHES_TO_MANUAL_MODE)) {
          config->ParameterFlags
              .TemperatureSetpointChangeSwitchesToManualModeHidden = 1;
        }
      }
    }
  }

  fields = cJSON_GetObjectItem(
      properties_root,
      field_map.at(FIELD_READONLY_TEMPERATURE_CONFIG_FIELDS).c_str());

  if (fields && cJSON_IsArray(fields)) {
    result = true;

    for (int a = 0; a < cJSON_GetArraySize(fields); a++) {
      cJSON *item = cJSON_GetArrayItem(fields, a);
      if (item && cJSON_IsString(item)) {
        char *str = cJSON_GetStringValue(item);
        if (!str) {
          continue;
        }

        if (str == temperatures_map.at(TEMPERATURE_FREEZE_PROTECTION)) {
          config->ParameterFlags.TemperaturesFreezeProtectionReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_ECO)) {
          config->ParameterFlags.TemperaturesEcoReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_COMFORT)) {
          config->ParameterFlags.TemperaturesComfortReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_BOOST)) {
          config->ParameterFlags.TemperaturesBoostReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_HEAT_PROTECTION)) {
          config->ParameterFlags.TemperaturesHeatProtectionReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_HISTERESIS)) {
          config->ParameterFlags.TemperaturesHisteresisReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_BELOW_ALARM)) {
          config->ParameterFlags.TemperaturesBelowAlarmReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_ABOVE_ALARM)) {
          config->ParameterFlags.TemperaturesAboveAlarmReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_AUX_MIN_SETPOINT)) {
          config->ParameterFlags.TemperaturesAuxMinSetpointReadonly = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_AUX_MAX_SETPOINT)) {
          config->ParameterFlags.TemperaturesAuxMaxSetpointReadonly = 1;
        }
      }
    }
  }

  fields = cJSON_GetObjectItem(
      properties_root,
      field_map.at(FIELD_HIDDEN_TEMPERATURE_CONFIG_FIELDS).c_str());

  if (fields && cJSON_IsArray(fields)) {
    result = true;

    for (int a = 0; a < cJSON_GetArraySize(fields); a++) {
      cJSON *item = cJSON_GetArrayItem(fields, a);
      if (item && cJSON_IsString(item)) {
        char *str = cJSON_GetStringValue(item);
        if (!str) {
          continue;
        }

        if (str == temperatures_map.at(TEMPERATURE_FREEZE_PROTECTION)) {
          config->ParameterFlags.TemperaturesFreezeProtectionHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_ECO)) {
          config->ParameterFlags.TemperaturesEcoHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_COMFORT)) {
          config->ParameterFlags.TemperaturesComfortHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_BOOST)) {
          config->ParameterFlags.TemperaturesBoostHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_HEAT_PROTECTION)) {
          config->ParameterFlags.TemperaturesHeatProtectionHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_HISTERESIS)) {
          config->ParameterFlags.TemperaturesHisteresisHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_BELOW_ALARM)) {
          config->ParameterFlags.TemperaturesBelowAlarmHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_ABOVE_ALARM)) {
          config->ParameterFlags.TemperaturesAboveAlarmHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_AUX_MIN_SETPOINT)) {
          config->ParameterFlags.TemperaturesAuxMinSetpointHidden = 1;
        }

        if (str == temperatures_map.at(TEMPERATURE_AUX_MAX_SETPOINT)) {
          config->ParameterFlags.TemperaturesAuxMaxSetpointHidden = 1;
        }
      }
    }
  }

  return result;
}
