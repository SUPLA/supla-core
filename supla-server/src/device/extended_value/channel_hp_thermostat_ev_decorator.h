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

#ifndef CHANNEL_HP_THERMOSTAT_EXTENDED_VALUE_DECORATOR_H_
#define CHANNEL_HP_THERMOSTAT_EXTENDED_VALUE_DECORATOR_H_

#include <string>

#include "device/extended_value/channel_thermostat_extended_value.h"

#define HP_STATUS_POWERON 0x1
#define HP_STATUS_PROGRAMMODE 0x4
#define HP_STATUS_HEATING 0x80

class supla_channel_hp_thermostat_ev_decorator {
 private:
  supla_channel_thermostat_extended_value *ev;

 public:
  explicit supla_channel_hp_thermostat_ev_decorator(
      supla_channel_thermostat_extended_value *ev);
  virtual ~supla_channel_hp_thermostat_ev_decorator(void);

  std::string get_home_assistant_mode(void);
  std::string get_home_assistant_action(void);
  unsigned char get_hvac_mode(void);
  unsigned short get_hvac_flags(void);
  int get_state_flags(void);
  bool is_program_mode(void);
  bool is_heating(void);
};

#endif /*CHANNEL_HP_THERMOSTAT_EXTENDED_VALUE_DECORATOR_H_*/
