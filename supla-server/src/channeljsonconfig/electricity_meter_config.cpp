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

#include "device/extended_value/channel_em_extended_value.h"
#include "srpc/srpc.h"

const _emc_map_t electricity_meter_config::map[] = {
    {.var = EM_VAR_FORWARD_ACTIVE_ENERGY, .str = "forwardActiveEnergy"},
    {.var = EM_VAR_REVERSE_ACTIVE_ENERGY, .str = "reverseActiveEnergy"},
    {.var = EM_VAR_FORWARD_REACTIVE_ENERGY, .str = "forwardReactiveEnergy"},
    {.var = EM_VAR_REVERSE_REACTIVE_ENERGY, .str = "reverseReactiveEnergy"},
    {.var = EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED,
     .str = "forwardActiveEnergyBalanced"},
    {.var = EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED,
     .str = "reverseActiveEnergyBalanced"}};

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

electricity_meter_config::electricity_meter_config(void)
    : channel_json_config() {}

electricity_meter_config::electricity_meter_config(channel_json_config *root)
    : channel_json_config(root) {}

int electricity_meter_config::get_map_size(void) {
  return sizeof(map) / sizeof(_emc_map_t);
}

int electricity_meter_config::get_map_key(int index) { return map[index].var; }

const char *electricity_meter_config::get_map_str(int index) {
  return map[index].str.c_str();
}

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
      result |= key_with_string(cJSON_GetStringValue(item));
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
  int msize = get_map_size();

  for (int a = 0; a < msize; a++) {
    if ((measured_values & get_map_key(a)) &&
        (available_counters & get_map_key(a)) == 0) {
      if (!available) {
        available = cJSON_AddArrayToObject(root, counters_available_key);

        if (!available) {
          return false;
        }
      }

      cJSON *jstr = cJSON_CreateString(get_map_str(a));
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

_supla_int64_t electricity_meter_config::get_initial_value(
    int var, unsigned char phase, bool *initial_value_for_all_phases) {
  if (initial_value_for_all_phases) {
    *initial_value_for_all_phases = true;
  }

  if (!key_exists(var) || (get_available_counters() & var) == 0) {
    return 0;
  }

  cJSON *root = get_user_root();
  if (!root) {
    return 0;
  }

  cJSON *initial_values = cJSON_GetObjectItem(root, em_initial_values_key);
  if (initial_values) {
    cJSON *initial_value =
        cJSON_GetObjectItem(initial_values, string_with_key(var));
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
    int flags, TElectricityMeter_ExtendedValue_V2 *em_ev) {
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
