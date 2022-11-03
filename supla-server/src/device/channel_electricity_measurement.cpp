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

#include "channel_electricity_measurement.h"

#include <string.h>

#include "channel_ic_measurement.h"
#include "safearray.h"
#include "srpc/srpc.h"

supla_channel_electricity_measurement::supla_channel_electricity_measurement(
    int ChannelId, TElectricityMeter_ExtendedValue_V2 *em_ev, int Param2,
    const char *TextParam1) {
  this->ChannelId = ChannelId;
  assign(Param2, TextParam1, em_ev);
}

void supla_channel_electricity_measurement::assign(
    int Param2, const char *TextParam1,
    TElectricityMeter_ExtendedValue_V2 *em_ev) {
  if (em_ev == NULL) {
    memset(&this->em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V2));
  } else {
    memcpy(&this->em_ev, em_ev, sizeof(TElectricityMeter_ExtendedValue_V2));

    if (this->em_ev.m_count == 0) {
      memset(this->em_ev.m, 0, sizeof(TElectricityMeter_Measurement));
    }
  }

  supla_channel_electricity_measurement::set_costs(Param2, TextParam1,
                                                   &this->em_ev);
}

int supla_channel_electricity_measurement::getChannelId(void) {
  return ChannelId;
}

void supla_channel_electricity_measurement::getMeasurement(
    TElectricityMeter_ExtendedValue_V2 *em_ev) {
  if (em_ev) {
    memcpy(em_ev, &this->em_ev, sizeof(TElectricityMeter_ExtendedValue_V2));
  }
}

void supla_channel_electricity_measurement::getCurrency(char currency[4]) {
  memcpy(currency, this->em_ev.currency, 3);
  currency[3] = 0;
}

// static
void supla_channel_electricity_measurement::set_costs(
    int Param2, const char *TextParam1,
    TElectricityMeter_ExtendedValue_V2 *em_ev) {
  double sum = em_ev->total_forward_active_energy[0] * 0.00001;
  sum += em_ev->total_forward_active_energy[1] * 0.00001;
  sum += em_ev->total_forward_active_energy[2] * 0.00001;

  supla_channel_ic_measurement::get_cost_and_currency(
      TextParam1, Param2, em_ev->currency, &em_ev->total_cost,
      &em_ev->price_per_unit, sum);

  if (em_ev->measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED) {
    supla_channel_ic_measurement::get_cost_and_currency(
        TextParam1, Param2, em_ev->currency, &em_ev->total_cost_balanced,
        &em_ev->price_per_unit,
        em_ev->total_forward_active_energy_balanced * 0.00001);
  } else {
    em_ev->total_cost_balanced = 0;
  }
}

// static
bool supla_channel_electricity_measurement::update_cev(
    TSuplaChannelExtendedValue *ev, int Param2, const char *TextParam1) {
  if (ev->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2) {
    TElectricityMeter_ExtendedValue_V2 em_ev;
    if (srpc_evtool_v2_extended2emextended(ev, &em_ev)) {
      supla_channel_electricity_measurement::set_costs(Param2, TextParam1,
                                                       &em_ev);

      srpc_evtool_v2_emextended2extended(&em_ev, ev);
      return true;
    }
  }

  return false;
}
