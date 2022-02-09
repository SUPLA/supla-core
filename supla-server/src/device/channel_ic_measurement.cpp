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

#include "channel_ic_measurement.h"

#include <stdio.h>
#include <string.h>

#include "safearray.h"
#include "srpc.h"

supla_channel_ic_measurement::supla_channel_ic_measurement(
    int ChannelId, int Func, TDS_ImpulseCounter_Value *ic_val,
    const char *TextParam1, const char *TextParam2, int Param2, int Param3) {
  this->ChannelId = ChannelId;
  this->totalCost = 0;
  this->pricePerUnit = 0;
  this->currency[0] = 0;
  this->impulsesPerUnit = 0;
  this->customUnit[0] = 0;

  if (TextParam2 && strnlen(TextParam2, 9) < 9) {
    strncpy(this->customUnit, TextParam2, 9);
    this->customUnit[8] = 0;
  }

  supla_channel_ic_measurement::set_default_unit(Func, this->customUnit);

  if (Param3 > 0) {
    this->impulsesPerUnit = Param3;
  }

  this->counter = ic_val->counter;

  this->calculatedValue = supla_channel_ic_measurement::get_calculated_i(
      this->impulsesPerUnit, this->counter);

  supla_channel_ic_measurement::get_cost_and_currency(
      TextParam1, Param2, this->currency, &this->totalCost, &this->pricePerUnit,
      supla_channel_ic_measurement::get_calculated_d(this->impulsesPerUnit,
                                                     this->counter));

  this->currency[3] = 0;
}

int supla_channel_ic_measurement::getChannelId(void) { return ChannelId; }

_supla_int_t supla_channel_ic_measurement::getTotalCost(void) {
  return totalCost;
}

_supla_int_t supla_channel_ic_measurement::getPricePerUnit(void) {
  return pricePerUnit;
}

const char *supla_channel_ic_measurement::getCurrency(void) { return currency; }

const char *supla_channel_ic_measurement::getCustomUnit(void) {
  return customUnit;
}

_supla_int_t supla_channel_ic_measurement::getImpulsesPerUnit(void) {
  return impulsesPerUnit;
}

unsigned _supla_int64_t supla_channel_ic_measurement::getCounter(void) {
  return counter;
}

unsigned _supla_int64_t supla_channel_ic_measurement::getCalculatedValue(void) {
  return calculatedValue;
}

// static
void supla_channel_ic_measurement::set_default_unit(int Func, char unit[9]) {
  if (strnlen(unit, 9) == 0) {
    switch (Func) {
      case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
        snprintf(unit, 9, "kWh");  // NOLINT
        break;
      case SUPLA_CHANNELFNC_IC_GAS_METER:
      case SUPLA_CHANNELFNC_IC_WATER_METER:
        // UTF(³) == 0xc2b3
        snprintf(unit, 9, "m%c%c", 0xc2, 0xb3);  // NOLINT
        break;
      case SUPLA_CHANNELFNC_IC_HEAT_METER:
        snprintf(unit, 9, "GJ");  // NOLINT
        break;
    }
  }
}

// static
bool supla_channel_ic_measurement::update_cev(
    TSC_SuplaChannelExtendedValue *cev, int Func, int Param2, int Param3,
    const char *TextParam1, const char *TextParam2) {
  TSC_ImpulseCounter_ExtendedValue ic_ev;
  if (srpc_evtool_v1_extended2icextended(&cev->value, &ic_ev)) {
    ic_ev.calculated_value = 0;
    ic_ev.custom_unit[0] = 0;
    ic_ev.impulses_per_unit = 0;

    if (TextParam2 && strnlen(TextParam2, 9) < 9) {
      strncpy(ic_ev.custom_unit, TextParam2, 8);
      ic_ev.custom_unit[8] = 0;
    }

    supla_channel_ic_measurement::set_default_unit(Func, ic_ev.custom_unit);

    if (Param3 > 0) {
      ic_ev.impulses_per_unit = Param3;
    }

    ic_ev.calculated_value = supla_channel_ic_measurement::get_calculated_i(
        ic_ev.impulses_per_unit, ic_ev.counter);

    supla_channel_ic_measurement::get_cost_and_currency(
        TextParam1, Param2, ic_ev.currency, &ic_ev.total_cost,
        &ic_ev.price_per_unit,
        supla_channel_ic_measurement::get_calculated_d(ic_ev.impulses_per_unit,
                                                       ic_ev.counter));

    srpc_evtool_v1_icextended2extended(&ic_ev, &cev->value);
    return true;
  }
  return false;
}

// static
double supla_channel_ic_measurement::get_calculated_d(
    _supla_int_t impulses_per_unit, unsigned _supla_int64_t counter) {
  return supla_channel_ic_measurement::get_calculated_i(impulses_per_unit,
                                                        counter) /
         1000.00;
}

// static
_supla_int64_t supla_channel_ic_measurement::get_calculated_i(
    _supla_int_t impulses_per_unit, unsigned _supla_int64_t counter) {
  return impulses_per_unit > 0 ? counter * 1000 / impulses_per_unit : 0;
}

// static
void supla_channel_ic_measurement::get_cost_and_currency(
    const char *TextParam1, int Param2, char currency[3],
    _supla_int_t *total_cost, _supla_int_t *price_per_unit, double count) {
  currency[0] = 0;
  *total_cost = 0;
  *price_per_unit = 0;

  if (TextParam1 && strlen(TextParam1) == 3) {
    memcpy(currency, TextParam1, 3);
  }

  if (Param2 > 0) {
    *price_per_unit = Param2;
    // *total_cost = (double)(Param2 * 0.0001 * count) * 100;
    *total_cost = (double)(Param2 * 0.01 * count);
  }
}

// static
char supla_channel_ic_measurement::icarr_clean(void *ptr) {
  delete (supla_channel_ic_measurement *)ptr;
  return 1;
}

// static
void supla_channel_ic_measurement::free(void *icarr) {
  safe_array_clean(icarr, icarr_clean);
  safe_array_free(icarr);
}
