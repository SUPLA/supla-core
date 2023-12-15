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

#include "channel_hp_thermostat_ev_decorator.h"

#include <stdlib.h>
#include <string.h>

using std::string;

supla_channel_hp_thermostat_ev_decorator::
    supla_channel_hp_thermostat_ev_decorator(
        supla_channel_thermostat_extended_value *ev) {
  this->ev = ev;
}

supla_channel_hp_thermostat_ev_decorator::
    ~supla_channel_hp_thermostat_ev_decorator(void) {}

int supla_channel_hp_thermostat_ev_decorator::get_state_flags(void) {
  return ev ? ev->get_flags(4) : 0;
}

string supla_channel_hp_thermostat_ev_decorator::get_home_assistant_mode(void) {
  short flags = get_state_flags();

  if (flags & HP_STATUS_POWERON) {
    if (flags & HP_STATUS_PROGRAMMODE) {
      return "auto";
    } else if (flags & HP_STATUS_POWERON) {
      return "heat";
    }
  }

  return "off";
}

string supla_channel_hp_thermostat_ev_decorator::get_home_assistant_action(
    void) {
  short flags = get_state_flags();

  if (flags & HP_STATUS_POWERON) {
    return flags & HP_STATUS_HEATING ? "heating" : "idle";
  }
  return "off";
}

unsigned char supla_channel_hp_thermostat_ev_decorator::get_hvac_mode(void) {
  short flags = get_state_flags();
  return flags & HP_STATUS_POWERON ? SUPLA_HVAC_MODE_HEAT : SUPLA_HVAC_MODE_OFF;
}

unsigned short supla_channel_hp_thermostat_ev_decorator::get_hvac_flags(void) {
  unsigned short result = 0;

  short flags = get_state_flags();

  if (flags & HP_STATUS_HEATING) {
    result |= SUPLA_HVAC_VALUE_FLAG_HEATING;
  }

  if (flags & HP_STATUS_PROGRAMMODE) {
    result |= SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE;
  }

  return result;
}

bool supla_channel_hp_thermostat_ev_decorator::is_heating(void) {
  short flags = get_state_flags();
  return (flags & HP_STATUS_POWERON) && (flags & HP_STATUS_HEATING);
}
