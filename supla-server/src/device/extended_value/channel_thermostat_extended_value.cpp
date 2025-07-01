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

#include "channel_thermostat_extended_value.h"

#include <stdlib.h>
#include <string.h>

#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "log.h"

supla_channel_thermostat_extended_value::
    supla_channel_thermostat_extended_value(
        const TSuplaChannelExtendedValue *value)
    : supla_abstract_channel_extended_value(value) {}

supla_channel_thermostat_extended_value::
    supla_channel_thermostat_extended_value(
        const supla_abstract_channel_extended_value *value)
    : supla_abstract_channel_extended_value(value) {}

supla_channel_thermostat_extended_value::
    ~supla_channel_thermostat_extended_value(void) {}

short supla_channel_thermostat_extended_value::get_flags(unsigned char idx) {
  if (get_value_size() >= 57) {  // 57 is the minimum number of bytes needed to
                                 // cover the Flags array.
    TThermostat_ExtendedValue *th =
        (TThermostat_ExtendedValue *)get_value_ptr()->value;

    if ((th->Fields & THERMOSTAT_FIELD_Flags) &&
        idx < sizeof(th->Flags) / sizeof(th->Flags[0])) {
      return th->Flags[idx];
    }
  }

  return 0;
}

supla_abstract_channel_extended_value *
supla_channel_thermostat_extended_value::copy(  // NOLINT
    void) {                                     // NOLINT
  return new supla_channel_thermostat_extended_value(this);
}

// static
bool supla_channel_thermostat_extended_value::is_ev_type_supported(char type) {
  return type == EV_TYPE_THERMOSTAT_DETAILS_V1;
}

bool supla_channel_thermostat_extended_value::get_vbt_value(
    _vbt_var_name_e var_name, double *value) {
  // Currently, only heatpol uses this, so we do not need to check the channel
  // function.

  supla_channel_hp_thermostat_ev_decorator decorator(this);

  switch (var_name) {
    case var_name_heating:
    case var_name_is_on:
      *value = decorator.is_heating() ? 1 : 0;
      break;
    default:
      return false;
  }

  return true;
}
