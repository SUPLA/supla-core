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

#include "channel_ic_extended_value.h"

#include <stdio.h>
#include <string.h>

#include "srpc/srpc.h"

using std::string;

supla_channel_ic_extended_value::supla_channel_ic_extended_value(
    int func, const TDS_ImpulseCounter_Value *ic_val, const char *text_param1,
    const char *text_param2, int param2, int param3)
    : supla_channel_extended_value(), supla_channel_billing_value() {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};

  if (!srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    ic_ev = {};
  }

  ic_ev.counter = ic_val ? ic_val->counter : 0;

  if (text_param2 && strnlen(text_param2, 9) < 9) {
    snprintf(ic_ev.custom_unit, sizeof(ic_ev.custom_unit), "%s", text_param2);
  }

  if (strnlen(ic_ev.custom_unit, 9) == 0) {
    switch (func) {
      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
        snprintf(ic_ev.custom_unit, 9, "kWh");  // NOLINT
        break;
      case SUPLA_CHANNELFNC_IC_GAS_METER:
      case SUPLA_CHANNELFNC_IC_WATER_METER:
        // UTF(³) == 0xc2b3
        snprintf(ic_ev.custom_unit, 9, "m%c%c", 0xc2, 0xb3);  // NOLINT
        break;
      case SUPLA_CHANNELFNC_IC_HEAT_METER:
        snprintf(ic_ev.custom_unit, 9, "GJ");  // NOLINT
        break;
    }
  }

  if (param3 > 0) {
    ic_ev.impulses_per_unit = param3;
  }

  ic_ev.calculated_value = ic_ev.impulses_per_unit > 0
                               ? ic_ev.counter * 1000 / ic_ev.impulses_per_unit
                               : 0;

  get_cost_and_currency(text_param1, param2, ic_ev.currency, &ic_ev.total_cost,
                        &ic_ev.price_per_unit,
                        ic_ev.calculated_value / 1000.00);

  TSuplaChannelExtendedValue ev = {};
  if (srpc_evtool_v1_icextended2extended(&ic_ev, &ev)) {
    TSuplaChannelExtendedValue *value = _realloc(ev.size);
    if (value) {
      memcpy(value, &ev, get_real_size());
    }
  }
}

supla_channel_ic_extended_value::~supla_channel_ic_extended_value(void) {}

_supla_int_t supla_channel_ic_extended_value::get_total_cost(void) {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};
  if (srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    return ic_ev.total_cost;
  }

  return 0;
}

_supla_int_t supla_channel_ic_extended_value::get_price_per_unit(void) {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};
  if (srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    return ic_ev.price_per_unit;
  }

  return 0;
}

string supla_channel_ic_extended_value::get_currency(void) {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};
  if (srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    char currency[4] = {};
    memcpy(currency, ic_ev.currency, 3);
    return currency;
  }

  return "";
}

string supla_channel_ic_extended_value::get_custom_unit(void) {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};
  if (srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    return ic_ev.custom_unit;
  }

  return "";
}

_supla_int_t supla_channel_ic_extended_value::get_impulses_per_unit(void) {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};
  if (srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    return ic_ev.impulses_per_unit;
  }

  return 0;
}

unsigned _supla_int64_t supla_channel_ic_extended_value::get_counter(void) {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};
  if (srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    return ic_ev.counter;
  }

  return 0;
}

unsigned _supla_int64_t
supla_channel_ic_extended_value::get_calculated_value(void) {
  TSC_ImpulseCounter_ExtendedValue ic_ev = {};
  if (srpc_evtool_v1_extended2icextended(get_value_ptr(), &ic_ev)) {
    return ic_ev.calculated_value;
  }

  return 0;
}

// static
bool supla_channel_ic_extended_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      return true;
  }

  return false;
}
