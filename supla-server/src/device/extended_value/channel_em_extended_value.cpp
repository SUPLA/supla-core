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

#include "channel_em_extended_value.h"

#include <string.h>

#include "analyzer/voltage_analyzers.h"
#include "db/db_access_provider.h"
#include "device/device_dao.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

using std::string;

supla_channel_em_extended_value::supla_channel_em_extended_value(
    const TSuplaChannelExtendedValue *value, const char *text_param1,
    int param2)
    : supla_channel_extended_value(), supla_channel_billing_value() {
  TElectricityMeter_ExtendedValue_V2 v2 = {};

  if (value) {
    if (value->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1) {
      TElectricityMeter_ExtendedValue v1 = {};
      if (!srpc_evtool_v1_extended2emextended(value, &v1) ||
          !srpc_evtool_emev_v1to2(&v1, &v2)) {
        return;
      }
    } else if (value->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2) {
      if (!srpc_evtool_v2_extended2emextended(value, &v2)) {
        return;
      }
    }
  }

  set_raw_value(&v2, text_param1, &param2);
}

supla_channel_em_extended_value::supla_channel_em_extended_value(
    const TElectricityMeter_ExtendedValue_V2 *value, const char *text_param1,
    int param2) {
  set_raw_value(value, text_param1, &param2);
}

void supla_channel_em_extended_value::set_raw_value(
    const TSuplaChannelExtendedValue *value) {
  return supla_channel_extended_value::set_raw_value(value);
}

void supla_channel_em_extended_value::set_raw_value(
    const TElectricityMeter_ExtendedValue_V2 *value) {
  set_raw_value(value, nullptr, nullptr);
}

void supla_channel_em_extended_value::set_raw_value(
    const TElectricityMeter_ExtendedValue_V2 *_value, const char *text_param1,
    int *param2) {
  TSuplaChannelExtendedValue new_value = {};
  if (!srpc_evtool_v2_emextended2extended(_value, &new_value)) {
    return;
  }

  TElectricityMeter_ExtendedValue_V2 *value =
      (TElectricityMeter_ExtendedValue_V2 *)new_value.value;

  double sum = value->total_forward_active_energy[0] * 0.00001;
  sum += value->total_forward_active_energy[1] * 0.00001;
  sum += value->total_forward_active_energy[2] * 0.00001;

  if (param2) {
    value->price_per_unit = *param2;
  }

  if (text_param1) {
    supla_channel_billing_value::get_currency(value->currency, text_param1);
  }

  value->total_cost = get_cost(value->price_per_unit, sum);

  if (value->measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED) {
    value->total_cost_balanced =
        get_cost(value->price_per_unit,
                 value->total_forward_active_energy_balanced * 0.00001);
  } else {
    value->total_cost_balanced = 0;
  }

  set_raw_value(&new_value);
}

supla_channel_em_extended_value::~supla_channel_em_extended_value(void) {}

double supla_channel_em_extended_value::get_total_cost(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.total_cost * 0.01;
  }
  return 0.0;
}

double supla_channel_em_extended_value::get_total_cost_balanced(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.total_cost_balanced * 0.01;
  }
  return 0.0;
}

double supla_channel_em_extended_value::get_price_per_unit(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.price_per_unit * 0.0001;
  }
  return 0.0;
}

string supla_channel_em_extended_value::get_currency(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    char currency[4] = {};
    memcpy(currency, em_ev.currency, 3);
    return currency;
  }
  return "";
}

_supla_int_t supla_channel_em_extended_value::get_measured_values(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.measured_values;
  }
  return 0;
}

double supla_channel_em_extended_value::get_voltage(int phase) {
  if (phase >= 1 && phase <= 3) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
        em_ev.m_count > 0) {
      return em_ev.m[0].voltage[phase - 1] * 0.01;
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_voltage_avg(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
      em_ev.m_count > 0) {
    short divider = 0;
    double sum = 0.0;
    for (unsigned char a = 0; a < 3; a++) {
      if (em_ev.m[0].voltage[a]) {
        sum += em_ev.m[0].voltage[a] * 0.01;
        divider++;
      }
    }

    if (divider) {
      return sum / divider;
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_current(int phase) {
  if (phase >= 1 && phase <= 3) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
        em_ev.m_count > 0) {
      return em_ev.m[0].current[phase - 1] *
             (em_ev.measured_values & EM_VAR_CURRENT_OVER_65A ? 0.01 : 0.001);
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_current_sum(void) {
  double sum = 0.0;
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
      em_ev.m_count > 0) {
    for (unsigned char a = 0; a < 3; a++) {
      sum += em_ev.m[0].current[a] *
             (em_ev.measured_values & EM_VAR_CURRENT_OVER_65A ? 0.01 : 0.001);
    }
  }

  return sum;
}

double supla_channel_em_extended_value::get_power_active(int phase) {
  if (phase >= 1 && phase <= 3) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
        em_ev.m_count > 0) {
      return em_ev.m[0].power_active[phase - 1] *
             (em_ev.measured_values & EM_VAR_POWER_ACTIVE_KW ? 0.01 : 0.00001);
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_power_active_sum(void) {
  double sum = 0.0;
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
      em_ev.m_count > 0) {
    for (unsigned char a = 0; a < 3; a++) {
      sum += em_ev.m[0].power_active[a] *
             (em_ev.measured_values & EM_VAR_POWER_ACTIVE_KW ? 0.01 : 0.00001);
    }
  }

  return sum;
}

double supla_channel_em_extended_value::get_power_reactive(int phase) {
  if (phase >= 1 && phase <= 3) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
        em_ev.m_count > 0) {
      return em_ev.m[0].power_reactive[phase - 1] *
             (em_ev.measured_values & EM_VAR_POWER_REACTIVE_KVAR ? 0.01
                                                                 : 0.00001);
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_power_reactive_sum(void) {
  double sum = 0.0;
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
      em_ev.m_count > 0) {
    for (unsigned char a = 0; a < 3; a++) {
      sum +=
          em_ev.m[0].power_reactive[a] *
          (em_ev.measured_values & EM_VAR_POWER_REACTIVE_KVAR ? 0.01 : 0.00001);
    }
  }

  return sum;
}

double supla_channel_em_extended_value::get_power_apparent(int phase) {
  if (phase >= 1 && phase <= 3) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
        em_ev.m_count > 0) {
      return em_ev.m[0].power_apparent[phase - 1] *
             (em_ev.measured_values & EM_VAR_POWER_APPARENT_KVA ? 0.01
                                                                : 0.00001);
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_power_apparent_sum(void) {
  double sum = 0.0;
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev) &&
      em_ev.m_count > 0) {
    for (unsigned char a = 0; a < 3; a++) {
      sum +=
          em_ev.m[0].power_apparent[a] *
          (em_ev.measured_values & EM_VAR_POWER_APPARENT_KVA ? 0.01 : 0.00001);
    }
  }

  return sum;
}

double supla_channel_em_extended_value::get_fae(int phase) {
  if (phase >= 1 && phase <= 3) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
      return em_ev.total_forward_active_energy[phase - 1] * 0.00001;
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_fae_sum(void) {
  double sum = 0.0;

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    for (unsigned char a = 0; a < 3; a++) {
      sum += em_ev.total_forward_active_energy[a] * 0.00001;
    }
  }

  return sum;
}

double supla_channel_em_extended_value::get_fae_balanced(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.total_forward_active_energy_balanced * 0.00001;
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_rae(int phase) {
  if (phase >= 1 && phase <= 3) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
      return em_ev.total_reverse_active_energy[phase - 1] * 0.00001;
    }
  }

  return 0.0;
}

double supla_channel_em_extended_value::get_rae_sum(void) {
  double sum = 0.0;

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    for (unsigned char a = 0; a < 3; a++) {
      sum += em_ev.total_reverse_active_energy[a] * 0.00001;
    }
  }

  return sum;
}

double supla_channel_em_extended_value::get_rae_balanced(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.total_reverse_active_energy_balanced * 0.00001;
  }

  return 0.0;
}

bool supla_channel_em_extended_value::get_raw_value(
    TSuplaChannelExtendedValue *value) {
  return supla_channel_extended_value::get_raw_value(value);
}

bool supla_channel_em_extended_value::get_raw_value(
    TElectricityMeter_ExtendedValue_V2 *value) {
  if (!value) {
    return false;
  }

  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), value)) {
    return true;
  }

  memset(value, 0, sizeof(TElectricityMeter_ExtendedValue_V2));
  return false;
}

supla_channel_extended_value *supla_channel_em_extended_value::
    supla_channel_em_extended_value::copy(  // NOLINT
        void) {                             // NOLINT
  supla_channel_em_extended_value *result = new supla_channel_em_extended_value(
      (const TSuplaChannelExtendedValue *)nullptr, nullptr, 0);
  result->set_raw_value(get_value_ptr());
  return result;
}

// static
bool supla_channel_em_extended_value::is_function_supported(int func) {
  return func == SUPLA_CHANNELFNC_ELECTRICITY_METER;
}

// static
bool supla_channel_em_extended_value::is_ev_type_supported(char type) {
  return type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1 ||
         type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2;
}
