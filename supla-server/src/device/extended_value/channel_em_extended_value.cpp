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

#include "srpc/srpc.h"

using std::string;

supla_channel_em_extended_value::supla_channel_em_extended_value(
    const TSuplaChannelExtendedValue *value, const char *text_param1,
    int param2)
    : supla_channel_extended_value(), supla_channel_billing_value() {
  TElectricityMeter_ExtendedValue_V2 v2 = {};

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
  } else {
    return;
  }

  init(&v2, text_param1, param2);
}

supla_channel_em_extended_value::supla_channel_em_extended_value(
    const TElectricityMeter_ExtendedValue_V2 *value, const char *text_param1,
    int param2) {
  TElectricityMeter_ExtendedValue_V2 copy = {};
  if (value) {
    memcpy(&copy, value, sizeof(TElectricityMeter_ExtendedValue_V2));
  }
  init(&copy, text_param1, param2);
}

void supla_channel_em_extended_value::init(
    TElectricityMeter_ExtendedValue_V2 *value, const char *text_param1,
    int param2) {
  double sum = value->total_forward_active_energy[0] * 0.00001;
  sum += value->total_forward_active_energy[1] * 0.00001;
  sum += value->total_forward_active_energy[2] * 0.00001;

  get_cost_and_currency(text_param1, param2, value->currency,
                        &value->total_cost, &value->price_per_unit, sum);

  if (value->measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED) {
    get_cost_and_currency(
        text_param1, param2, value->currency, &value->total_cost_balanced,
        &value->price_per_unit,
        value->total_forward_active_energy_balanced * 0.00001);
  } else {
    value->total_cost_balanced = 0;
  }

  TSuplaChannelExtendedValue new_value = {};

  if (srpc_evtool_v2_emextended2extended(value, &new_value)) {
    set_raw_value(&new_value);
  }
}

supla_channel_em_extended_value::~supla_channel_em_extended_value(void) {}

_supla_int_t supla_channel_em_extended_value::get_total_cost(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.total_cost;
  }
  return 0;
}

_supla_int_t supla_channel_em_extended_value::get_total_cost_balanced(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.total_cost_balanced;
  }
  return 0;
}

_supla_int_t supla_channel_em_extended_value::get_price_per_unit(void) {
  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(get_value_ptr(), &em_ev)) {
    return em_ev.price_per_unit;
  }
  return 0;
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

// static
bool supla_channel_em_extended_value::is_function_supported(int func) {
  return func == SUPLA_CHANNELFNC_ELECTRICITY_METER;
}

// static
bool supla_channel_em_extended_value::is_ev_type_supported(char type) {
  return type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1 ||
         type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2;
}
