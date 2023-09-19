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

#include "channel_hvac_value.h"

#include <string.h>

#include "proto.h"

supla_channel_hvac_value::supla_channel_hvac_value(void)
    : supla_channel_value() {}

supla_channel_hvac_value::supla_channel_hvac_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

void supla_channel_hvac_value::set_mode(unsigned char mode) {
  if (mode >= 0 && mode <= SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL) {
    ((THVACValue*)raw_value)->Mode = mode;
  }
}

unsigned char supla_channel_hvac_value::get_mode(void) {
  return ((THVACValue*)raw_value)->Mode;
}

short supla_channel_hvac_value::get_temperature_heat(void) {
  return ((THVACValue*)raw_value)->SetpointTemperatureHeat;
}

short supla_channel_hvac_value::get_temperature_cool(void) {
  return ((THVACValue*)raw_value)->SetpointTemperatureCool;
}

unsigned short supla_channel_hvac_value::get_flags(void) {
  return ((THVACValue*)raw_value)->Flags;
}

void supla_channel_hvac_value::set_temperature_heat(short temperature) {
  ((THVACValue*)raw_value)->SetpointTemperatureHeat = temperature;
  ((THVACValue*)raw_value)->Flags |=
      SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET;
}

void supla_channel_hvac_value::set_temperature_cool(short temperature) {
  ((THVACValue*)raw_value)->SetpointTemperatureCool = temperature;
  ((THVACValue*)raw_value)->Flags |=
      SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET;
}

void supla_channel_hvac_value::set_temperature_heat_to_null(void) {
  ((THVACValue*)raw_value)->SetpointTemperatureHeat = 0;
  ((THVACValue*)raw_value)->Flags ^=
      ((THVACValue*)raw_value)->Flags &
      SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET;
}

void supla_channel_hvac_value::set_temperature_cool_to_null(void) {
  ((THVACValue*)raw_value)->SetpointTemperatureCool = 0;
  ((THVACValue*)raw_value)->Flags ^=
      ((THVACValue*)raw_value)->Flags &
      SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET;
}

void supla_channel_hvac_value::set_flags(unsigned short flags) {
  ((THVACValue*)raw_value)->Flags = flags;
}

bool supla_channel_hvac_value::is_on(void) {
  switch (get_mode()) {
    case SUPLA_HVAC_MODE_OFF:
    case SUPLA_HVAC_MODE_HEAT:
    case SUPLA_HVAC_MODE_COOL:
    case SUPLA_HVAC_MODE_AUTO:
    case SUPLA_HVAC_MODE_FAN_ONLY:
    case SUPLA_HVAC_MODE_DRY:
      return true;
  }

  return false;
}

void supla_channel_hvac_value::turn_on(void) {
  set_mode(SUPLA_HVAC_MODE_CMD_TURN_ON);
}

void supla_channel_hvac_value::turn_off(void) { set_mode(SUPLA_HVAC_MODE_OFF); }

void supla_channel_hvac_value::toggle(void) {
  if (is_on() || get_mode() == SUPLA_HVAC_MODE_CMD_TURN_ON) {
    turn_off();
  } else {
    turn_on();
  }
}

void supla_channel_hvac_value::switch_to_manual(void) {
  set_mode(SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL);
}

// static
bool supla_channel_hvac_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_AUTO:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
    case SUPLA_CHANNELFNC_HVAC_DRYER:
    case SUPLA_CHANNELFNC_HVAC_FAN:
    case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
      return true;
  }

  return false;
}
