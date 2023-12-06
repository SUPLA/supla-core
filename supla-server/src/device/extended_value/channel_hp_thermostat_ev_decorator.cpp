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

string supla_channel_hp_thermostat_ev_decorator::get_home_assistant_mode(void) {
  short flags = ev ? ev->get_flags(4) : 0;

  if (flags & HP_STATUS_PROGRAMMODE) {
    return "auto";
  } else if (flags & HP_STATUS_POWERON) {
    return "heat";
  }
  return "off";
}

string supla_channel_hp_thermostat_ev_decorator::get_home_assistant_action(
    void) {
  short flags = ev ? ev->get_flags(4) : 0;

  if (flags & HP_STATUS_POWERON) {
    return flags & HP_STATUS_HEATING ? "heating" : "idle";
  }
  return "off";
}
