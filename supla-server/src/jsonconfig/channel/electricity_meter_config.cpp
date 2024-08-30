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

#include "electricity_meter_config.h"

#include <limits.h>

#include <vector>

#include "device/extended_value/channel_em_extended_value.h"
#include "srpc/srpc.h"

using std::map;
using std::string;
using std::vector;

const map<int, string> electricity_meter_config::em_var_map = {
    {EM_VAR_FORWARD_ACTIVE_ENERGY, "forwardActiveEnergy"},
    {EM_VAR_REVERSE_ACTIVE_ENERGY, "reverseActiveEnergy"},
    {EM_VAR_FORWARD_REACTIVE_ENERGY, "forwardReactiveEnergy"},
    {EM_VAR_REVERSE_REACTIVE_ENERGY, "reverseReactiveEnergy"},
    {EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, "forwardActiveEnergyBalanced"},
    {EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED, "reverseActiveEnergyBalanced"}};

#define FIELD_USED_CT_TYPE 1
#define FIELD_USED_PHASE_LED_TYPE 2
#define FIELD_PHASE_LED_PARAM1 3
#define FIELD_PHASE_LED_PARAM2 4
#define FIELD_AVAILABLE_CT_TYPES 5
#define FIELD_AVAILABLE_PHASE_LED_TYPES 6

const map<unsigned _supla_int16_t, string> electricity_meter_config::field_map =
    {{FIELD_USED_CT_TYPE, "usedCTType"},
     {FIELD_USED_PHASE_LED_TYPE, "usedPhaseLedType"},
     {FIELD_PHASE_LED_PARAM1, "phaseLedParam1"},
     {FIELD_PHASE_LED_PARAM2, "phaseLedParam2"},
     {FIELD_AVAILABLE_CT_TYPES, "availableCTTypes"},
     {FIELD_AVAILABLE_PHASE_LED_TYPES, "availablePhaseLedTypes"}};

// static
const char electricity_meter_config::counters_available_key[] =
    "countersAvailable";

// static
const char electricity_meter_config::em_initial_values_key[] =
    "electricityMeterInitialValues";

// static
const char electricity_meter_config::add_to_history_key[] = "addToHistory";

// static
const char electricity_meter_config::upper_voltage_threshold_key[] =
    "upperVoltageThreshold";

// static
const char electricity_meter_config::lower_voltage_threshold_key[] =
    "lowerVoltageThreshold";

// static
const char electricity_meter_config::disabled_phases_key[] = "disabledPhases";

// static
const char electricity_meter_config::voltage_logger_enabled_key[] =
    "voltageLoggerEnabled";

// static
const char electricity_meter_config::current_logger_enabled_key[] =
    "currentLoggerEnabled";

// static
const char electricity_meter_config::power_active_logger_enabled_key[] =
    "powerActiveLoggerEnabled";

electricity_meter_config::electricity_meter_config(void)
    : supla_json_config() {}

electricity_meter_config::electricity_meter_config(supla_json_config *root)
    : supla_json_config(root) {}

int electricity_meter_config::get_available_counters(void) {
  cJSON *root = get_properties_root();
  if (!root) {
    return 0;
  }

  cJSON *available = cJSON_GetObjectItem(root, counters_available_key);
  if (!available || !cJSON_IsArray(available)) {
    return 0;
  }

  int result = 0;
  int asize = cJSON_GetArraySize(available);

  for (int a = 0; a < asize; a++) {
    cJSON *item = cJSON_GetArrayItem(available, a);
    if (item && cJSON_IsString(item)) {
      for (auto it = em_var_map.cbegin(); it != em_var_map.cend(); ++it) {
        if (equal_ci(item, it->second.c_str())) {
          result |= it->first;
        }
      }
    }
  }

  return result;
}

bool electricity_meter_config::update_available_counters(int measured_values) {
  cJSON *root = get_properties_root();
  if (!root) {
    return false;
  }

  cJSON *available = cJSON_GetObjectItem(root, counters_available_key);

  if (available && !cJSON_IsArray(available)) {
    return false;
  }

  int available_counters = get_available_counters();

  bool result = false;

  for (auto it = em_var_map.cbegin(); it != em_var_map.cend(); ++it) {
    if ((measured_values & it->first) &&
        (available_counters & it->first) == 0) {
      if (!available) {
        available = cJSON_AddArrayToObject(root, counters_available_key);

        if (!available) {
          return false;
        }
      }

      cJSON *jstr = cJSON_CreateString(it->second.c_str());
      if (jstr) {
        cJSON_AddItemToArray(available, jstr);
        result = true;
      }
    }
  }

  return result;
}

bool electricity_meter_config::update_available_counters(
    supla_channel_em_extended_value *em) {
  if (em == NULL) {
    return false;
  }

  return update_available_counters(em->get_measured_values());
}

bool electricity_meter_config::should_be_added_to_history(void) {
  return get_bool(add_to_history_key);
}

double electricity_meter_config::get_upper_voltage_threshold(void) {
  return get_double(upper_voltage_threshold_key);
}

double electricity_meter_config::get_lower_voltage_threshold(void) {
  return get_double(lower_voltage_threshold_key);
}

int electricity_meter_config::electricity_meter_config::get_channel_user_flags(
    void) {
  int flags = 0;
  if (is_phase_disabled(1)) {
    flags |= SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED;
  }

  if (is_phase_disabled(2)) {
    flags |= SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED;
  }

  if (is_phase_disabled(3)) {
    flags |= SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED;
  }

  return flags;
}

bool electricity_meter_config::is_phase_disabled(unsigned char phase) {
  cJSON *root = get_user_root();
  if (!root) {
    return false;
  }

  cJSON *disabled = cJSON_GetObjectItem(root, disabled_phases_key);

  if (!disabled || !cJSON_IsArray(disabled)) {
    return false;
  }

  int asize = cJSON_GetArraySize(disabled);

  for (int a = 0; a < asize; a++) {
    cJSON *item = cJSON_GetArrayItem(disabled, a);
    if (item && cJSON_IsNumber(item) && item->valueint == phase) {
      return true;
    }
  }

  return false;
}

bool electricity_meter_config::is_voltage_logger_enabled(void) {
  return get_bool(voltage_logger_enabled_key);
}

bool electricity_meter_config::is_current_logger_enabled(void) {
  return get_bool(current_logger_enabled_key);
}

bool electricity_meter_config::is_power_active_logger_enabled(void) {
  return get_bool(power_active_logger_enabled_key);
}

_supla_int64_t electricity_meter_config::get_initial_value(
    int var, unsigned char phase, bool *initial_value_for_all_phases) {
  if (initial_value_for_all_phases) {
    *initial_value_for_all_phases = true;
  }

  if (em_var_map.find(var) == em_var_map.end() ||
      (get_available_counters() & var) == 0) {
    return 0;
  }

  cJSON *root = get_user_root();
  if (!root) {
    return 0;
  }

  cJSON *initial_values = cJSON_GetObjectItem(root, em_initial_values_key);
  if (initial_values) {
    auto it = em_var_map.find(var);

    cJSON *initial_value = cJSON_GetObjectItem(
        initial_values, it == em_var_map.end() ? nullptr : it->second.c_str());
    if (initial_value) {
      if (cJSON_IsObject(initial_value)) {
        if (initial_value_for_all_phases) {
          *initial_value_for_all_phases = false;
        }

        char phase_key[4] = {};
        snprintf(phase_key, sizeof(phase_key), "%i", phase);
        cJSON *phase_initial_value =
            cJSON_GetObjectItem(initial_value, phase_key);
        if (phase_initial_value && cJSON_IsNumber(phase_initial_value)) {
          return 100000.0 * phase_initial_value->valuedouble;
        }
      } else if (cJSON_IsNumber(initial_value)) {
        return 100000.0 * initial_value->valuedouble;
      }
    }
  }

  return 0;
}

_supla_int64_t electricity_meter_config::get_initial_value_for_all_phases(
    int var) {
  bool initial_value_for_all_phases = false;

  _supla_int64_t initial_value =
      get_initial_value(var, 1, &initial_value_for_all_phases);

  if (!initial_value_for_all_phases) {
    initial_value +=
        get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 2, nullptr);

    initial_value +=
        get_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, 3, nullptr);
  }

  return initial_value;
}

void electricity_meter_config::add_initial_value(
    _supla_int64_t initial_value, bool initial_value_for_all_phases,
    unsigned char phase, int flags, unsigned _supla_int64_t *value,
    _supla_int64_t *substracted) {
  if (phase < 1 || phase > 3) {
    return;
  }

  unsigned _supla_int64_t left = -1 - *value;

  unsigned char phase_count = 3;
  unsigned char first_supported_phase = 1;

  if (flags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED) {
    if (phase == 1) {
      return;
    }
    phase_count--;
    first_supported_phase = 2;
  }

  if (flags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED) {
    if (phase == 2) {
      return;
    }
    phase_count--;
    first_supported_phase = 3;
  }

  if (flags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED) {
    if (phase == 3) {
      return;
    }
    phase_count--;
  }

  _supla_int64_t addition = initial_value;
  if (initial_value_for_all_phases) {
    addition /= phase_count;

    if (first_supported_phase == phase) {
      addition += initial_value - (initial_value / phase_count * phase_count);
    }
  }

  if (addition < 0) {
    if ((unsigned _supla_int64_t)(addition * -1) > *value) {
      addition = *value * -1;
    }
    *value += addition;
    *substracted += addition;
  } else {
    *value += (unsigned _supla_int64_t)addition > left ? left : addition;
  }
}

void electricity_meter_config::add_initial_value(
    int var, unsigned char phase, int flags, unsigned _supla_int64_t *value) {
  bool initial_value_for_all_phases = false;
  _supla_int64_t initial_value =
      get_initial_value(var, phase, &initial_value_for_all_phases);
  _supla_int64_t substracted = 0;
  add_initial_value(initial_value, initial_value_for_all_phases, phase, flags,
                    value, &substracted);
}

void electricity_meter_config::add_initial_value(
    int var, unsigned _supla_int64_t *value) {
  _supla_int64_t initial_value = get_initial_value_for_all_phases(var);
  _supla_int64_t substracted = 0;
  add_initial_value(initial_value, false, 1, 0, value, &substracted);
}

void electricity_meter_config::add_initial_value(
    int var, int flags, unsigned _supla_int64_t value[]) {
  bool initial_value_for_all_phases = false;
  _supla_int64_t initial_value =
      get_initial_value(var, 1, &initial_value_for_all_phases);
  _supla_int64_t substracted = 0;
  bool calculate = true;

  while (calculate) {
    if (initial_value < 0) {
      if (value[0] == 0) {
        flags |= SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED;
      }

      if (value[1] == 0) {
        flags |= SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED;
      }

      if (value[2] == 0) {
        flags |= SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED;
      }
    }

    bool changed = false;

    for (unsigned char phase = 1; phase <= 3; phase++) {
      unsigned _supla_int64_t before = value[phase - 1];
      add_initial_value(initial_value, initial_value_for_all_phases, phase,
                        flags, &value[phase - 1], &substracted);

      if (!initial_value_for_all_phases && phase < 3) {
        initial_value = get_initial_value(var, phase + 1, nullptr);
      }

      if (before != value[phase - 1]) {
        changed = true;  // Infinity loop prevention
      }
    }

    if (initial_value_for_all_phases && changed && initial_value < 0 &&
        initial_value < substracted &&
        (value[0] > 0 || value[1] > 0 || value[2] > 0)) {
      initial_value -= substracted;
    } else {
      calculate = false;
    }
  }
}

void electricity_meter_config::add_initial_values(
    int flags, TElectricityMeter_ExtendedValue_V3 *em_ev) {
  if (!em_ev) {
    return;
  }

  add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, flags,
                    em_ev->total_forward_active_energy);

  add_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY, flags,
                    em_ev->total_reverse_active_energy);

  add_initial_value(EM_VAR_FORWARD_REACTIVE_ENERGY, flags,
                    em_ev->total_forward_reactive_energy);

  add_initial_value(EM_VAR_REVERSE_REACTIVE_ENERGY, flags,
                    em_ev->total_reverse_reactive_energy);

  add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED,
                    &em_ev->total_forward_active_energy_balanced);

  add_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED,
                    &em_ev->total_reverse_active_energy_balanced);
}

void electricity_meter_config::add_initial_value(
    unsigned _supla_int_t *total_forward_active_energy) {
  if (total_forward_active_energy) {
    _supla_int64_t initial_value =
        get_initial_value_for_all_phases(EM_VAR_FORWARD_ACTIVE_ENERGY) / 1000;

    if (initial_value < 0) {
      if (initial_value * -1 > (*total_forward_active_energy)) {
        initial_value = (*total_forward_active_energy) * -1;
      }
    } else {
      unsigned _supla_int64_t left = UINT_MAX - (*total_forward_active_energy);
      if ((unsigned _supla_int64_t)initial_value > left) {
        initial_value = left;
      }
    }

    *total_forward_active_energy += initial_value;
  }
}

string electricity_meter_config::ct_type_to_string(
    unsigned _supla_int64_t type) {
  switch (type) {
    case EM_CT_TYPE_100A_33mA:
      return "100A_33mA";
    case EM_CT_TYPE_200A_66mA:
      return "200A_66mA";
    case EM_CT_TYPE_400A_133mA:
      return "400A_133mA";
  }

  return "";
}

unsigned _supla_int64_t
electricity_meter_config::string_to_ct_type(const std::string &str) {
  if (str == "100A_33mA") {
    return EM_CT_TYPE_100A_33mA;
  } else if (str == "200A_66mA") {
    return EM_CT_TYPE_200A_66mA;
  } else if (str == "400A_133mA") {
    return EM_CT_TYPE_400A_133mA;
  }

  return 0;
}

string electricity_meter_config::led_type_to_string(
    unsigned _supla_int64_t type) {
  switch (type) {
    case EM_PHASE_LED_TYPE_OFF:
      return "OFF";
    case EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE:
      return "VOLTAGE_PRESENCE";
    case EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE_INVERTED:
      return "VOLTAGE_PRESENCE_INVERTED";
    case EM_PHASE_LED_TYPE_VOLTAGE_LEVEL:
      return "VOLTAGE_LEVEL";
    case EM_PHASE_LED_TYPE_POWER_ACTIVE_DIRECTION:
      return "POWER_ACTIVE_DIRECTION";
  }

  return "";
}

unsigned _supla_int64_t
electricity_meter_config::string_to_led_type(const std::string &str) {
  if (str == "OFF") {
    return EM_PHASE_LED_TYPE_OFF;
  } else if (str == "VOLTAGE_PRESENCE") {
    return EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE;
  } else if (str == "VOLTAGE_PRESENCE_INVERTED") {
    return EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE_INVERTED;
  } else if (str == "VOLTAGE_LEVEL") {
    return EM_PHASE_LED_TYPE_VOLTAGE_LEVEL;
  } else if (str == "POWER_ACTIVE_DIRECTION") {
    return EM_PHASE_LED_TYPE_POWER_ACTIVE_DIRECTION;
  }

  return 0;
}

void electricity_meter_config::set_config(
    TChannelConfig_ElectricityMeter *config) {
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

  set_item_value(user_root, field_map.at(FIELD_USED_CT_TYPE).c_str(),
                 cJSON_String, true, nullptr,
                 ct_type_to_string(config->UsedCTType).c_str(), 0);

  set_item_value(user_root, field_map.at(FIELD_USED_PHASE_LED_TYPE).c_str(),
                 cJSON_String, true, nullptr,
                 led_type_to_string(config->UsedPhaseLedType).c_str(), 0);

  set_item_value(user_root, field_map.at(FIELD_PHASE_LED_PARAM1).c_str(),
                 cJSON_Number, true, nullptr, nullptr, config->PhaseLedParam1);

  set_item_value(user_root, field_map.at(FIELD_PHASE_LED_PARAM2).c_str(),
                 cJSON_Number, true, nullptr, nullptr, config->PhaseLedParam2);

  vector<unsigned _supla_int64_t> ct_types = {
      EM_CT_TYPE_100A_33mA, EM_CT_TYPE_200A_66mA, EM_CT_TYPE_400A_133mA};
  cJSON *ct_types_arr = cJSON_CreateArray();

  for (auto it = ct_types.cbegin(); it != ct_types.cend(); ++it) {
    if (config->AvailableCTTypes & (*it)) {
      cJSON_AddItemToArray(ct_types_arr,
                           cJSON_CreateString(ct_type_to_string(*it).c_str()));
    }
  }

  set_item_value(properties_root,
                 field_map.at(FIELD_AVAILABLE_CT_TYPES).c_str(), cJSON_Object,
                 true, ct_types_arr, nullptr, 0);

  vector<unsigned _supla_int64_t> led_types = {
      EM_PHASE_LED_TYPE_OFF, EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE,
      EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE_INVERTED,
      EM_PHASE_LED_TYPE_VOLTAGE_LEVEL,
      EM_PHASE_LED_TYPE_POWER_ACTIVE_DIRECTION};
  cJSON *led_types_arr = cJSON_CreateArray();

  for (auto it = led_types.cbegin(); it != led_types.cend(); ++it) {
    if (config->AvailablePhaseLedTypes & (*it)) {
      cJSON_AddItemToArray(led_types_arr,
                           cJSON_CreateString(led_type_to_string(*it).c_str()));
    }
  }

  set_item_value(properties_root,
                 field_map.at(FIELD_AVAILABLE_PHASE_LED_TYPES).c_str(),
                 cJSON_Object, true, led_types_arr, nullptr, 0);
}

bool electricity_meter_config::get_config(
    TChannelConfig_ElectricityMeter *config) {
  if (!config) {
    return false;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return false;
  }

  *config = {};

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return false;
  }

  bool result = false;

  std::string str_value;
  if (get_string(user_root, field_map.at(FIELD_USED_CT_TYPE).c_str(),
                 &str_value)) {
    config->UsedCTType = string_to_ct_type(str_value);
    result = true;
  }

  if (get_string(user_root, field_map.at(FIELD_USED_PHASE_LED_TYPE).c_str(),
                 &str_value)) {
    config->UsedPhaseLedType = string_to_led_type(str_value);
    result = true;
  }

  double dbl_value = 0;
  if (get_double(user_root, field_map.at(FIELD_PHASE_LED_PARAM1).c_str(),
                 &dbl_value)) {
    config->PhaseLedParam1 = dbl_value;
    result = true;
  }

  if (get_double(user_root, field_map.at(FIELD_PHASE_LED_PARAM2).c_str(),
                 &dbl_value)) {
    config->PhaseLedParam2 = dbl_value;
    result = true;
  }

  for (char x = 0; x < 2; x++) {
    cJSON *available = cJSON_GetObjectItem(
        properties_root,
        field_map
            .at(x ? FIELD_AVAILABLE_CT_TYPES : FIELD_AVAILABLE_PHASE_LED_TYPES)
            .c_str());

    if (cJSON_IsArray(available)) {
      result = true;
      int asize = cJSON_GetArraySize(available);

      for (int a = 0; a < asize; a++) {
        cJSON *item = cJSON_GetArrayItem(available, a);
        if (item && cJSON_IsString(item)) {
          const char *str = cJSON_GetStringValue(item);
          if (x) {
            config->AvailableCTTypes |= string_to_ct_type(str);
          } else {
            config->AvailablePhaseLedTypes |= string_to_led_type(str);
          }
        }
      }
    }
  }

  return result;
}

void electricity_meter_config::merge(supla_json_config *_dst) {
  electricity_meter_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           true);
  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           field_map, true);
}
