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
#include "srpc.h"

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
    TSuplaChannelExtendedValue *ev) {
  if (ev == NULL) {
    return false;
  }

  if (ev->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1) {
    TElectricityMeter_ExtendedValue em_ev = {};

    if (srpc_evtool_v1_extended2emextended(ev, &em_ev) == 1) {
      return update_available_counters(em_ev.measured_values);
    }
  } else if (ev->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};

    if (srpc_evtool_v2_extended2emextended(ev, &em_ev) == 1) {
      return update_available_counters(em_ev.measured_values);
    }
  }

  return false;
}

unsigned _supla_int64_t electricity_meter_config::get_initial_value(int var) {
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
    if (initial_value && cJSON_IsNumber(initial_value)) {
      return (unsigned _supla_int64_t)100000.0 * initial_value->valuedouble;
    }
  }

  return 0;
}

void electricity_meter_config::add_initial_value(
    int var, unsigned char phase, int flags, unsigned _supla_int64_t *value) {
  if (phase > 3) {
    return;
  }

  if (phase == 0) {
    phase = 1;
    flags = SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
            SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED;
  }

  unsigned _supla_int64_t left = -1 - *value;
  unsigned _supla_int64_t initial_value = get_initial_value(var);

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

  unsigned _supla_int64_t addition = initial_value / phase_count;

  if (first_supported_phase == phase) {
    addition += initial_value - (initial_value / phase_count * phase_count);
  }

  *value += addition > left ? left : addition;
}

void electricity_meter_config::add_initial_values(
    int flags, TElectricityMeter_ExtendedValue_V2 *em_ev) {
  if (!em_ev) {
    return;
  }

  for (unsigned char phase = 1; phase <= 3; phase++) {
    add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY, phase, flags,
                      &em_ev->total_forward_active_energy[phase - 1]);

    add_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY, phase, flags,
                      &em_ev->total_reverse_active_energy[phase - 1]);

    add_initial_value(EM_VAR_FORWARD_REACTIVE_ENERGY, phase, flags,
                      &em_ev->total_forward_reactive_energy[phase - 1]);

    add_initial_value(EM_VAR_REVERSE_REACTIVE_ENERGY, phase, flags,
                      &em_ev->total_reverse_reactive_energy[phase - 1]);
  }

  add_initial_value(EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, 0, 0,
                    &em_ev->total_forward_active_energy_balanced);

  add_initial_value(EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED, 0, 0,
                    &em_ev->total_reverse_active_energy_balanced);
}

void electricity_meter_config::add_initial_values(
    int flags, TElectricityMeter_ExtendedValue *em_ev) {
  if (!em_ev) {
    return;
  }

  TElectricityMeter_ExtendedValue_V2 em_ev_v2 = {};
  srpc_evtool_emev_v1to2(em_ev, &em_ev_v2);
  add_initial_values(flags, &em_ev_v2);
  srpc_evtool_emev_v2to1(&em_ev_v2, em_ev);
}

void electricity_meter_config::add_initial_values(
    int flags, TSuplaChannelExtendedValue *ev) {
  if (ev == NULL) {
    return;
  }

  if (ev->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1) {
    TElectricityMeter_ExtendedValue em_ev = {};

    if (srpc_evtool_v1_extended2emextended(ev, &em_ev) == 1) {
      add_initial_values(flags, &em_ev);
      srpc_evtool_v1_emextended2extended(&em_ev, ev);
    }
  } else if (ev->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};

    if (srpc_evtool_v2_extended2emextended(ev, &em_ev) == 1) {
      add_initial_values(flags, &em_ev);
      srpc_evtool_v2_emextended2extended(&em_ev, ev);
    }
  }
}
