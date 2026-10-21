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

#include "channel_em_value.h"

#include <string.h>

#include <algorithm>
#include <cmath>
#include <limits>

#include "jsonconfig/channel/electricity_meter_config.h"

supla_channel_em_value::supla_channel_em_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {}

supla_channel_em_value::supla_channel_em_value(
    const TElectricityMeter_Value *value)
    : supla_abstract_channel_value() {
  memcpy(raw_value, value, sizeof(TElectricityMeter_Value));
}

supla_abstract_channel_value *supla_channel_em_value::copy(  // NOLINT
    void) const {                                            // NOLINT
  return new supla_channel_em_value(raw_value);
}

const TElectricityMeter_Value *supla_channel_em_value::get_em_value(void) {
  return (TElectricityMeter_Value *)raw_value;
}

bool supla_channel_em_value::set_phase_on(int phase, bool on) {
  if (phase < 1 || phase > 3) {
    return false;
  }

  TElectricityMeter_Value *value =
      reinterpret_cast<TElectricityMeter_Value *>(raw_value);
  unsigned char flag = 1 << (phase - 1);
  if (on) {
    value->flags |= flag;
  } else {
    value->flags &= ~flag;
  }
  return true;
}

bool supla_channel_em_value::set_total_forward_active_energy(double value) {
  if (!std::isfinite(value) || value < 0) {
    return false;
  }

  long double raw = std::round(static_cast<long double>(value) * 100.0L);
  raw = std::min(
      raw, static_cast<long double>(
               std::numeric_limits<unsigned _supla_int_t>::max()));
  reinterpret_cast<TElectricityMeter_Value *>(raw_value)
      ->total_forward_active_energy = static_cast<unsigned _supla_int_t>(raw);
  return true;
}

void supla_channel_em_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {
  electricity_meter_config config(json_config);
  config.add_initial_value(
      &reinterpret_cast<TElectricityMeter_Value *>(raw_value)
           ->total_forward_active_energy);
}

// static
bool supla_channel_em_value::is_function_supported(int func) {
  return func == SUPLA_CHANNELFNC_ELECTRICITY_METER;
}
