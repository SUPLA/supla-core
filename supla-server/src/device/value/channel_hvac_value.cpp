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

using std::string;

supla_channel_hvac_value::supla_channel_hvac_value(void)
    : supla_channel_value() {}

supla_channel_hvac_value::supla_channel_hvac_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

void supla_channel_hvac_value::clear(void) {
  memset(raw_value, 0, SUPLA_CHANNELVALUE_SIZE);
}

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

double supla_channel_hvac_value::get_temperature_heat_dbl(void) {
  return get_temperature_heat() / 100.00;
}

std::string supla_channel_hvac_value::get_temperature_heat_str(void) {
  char result[20] = {};
  snprintf(result, sizeof(result), "%.2f", get_temperature_heat_dbl());
  return result;
}

short supla_channel_hvac_value::get_temperature_cool(void) {
  return ((THVACValue*)raw_value)->SetpointTemperatureCool;
}

double supla_channel_hvac_value::get_temperature_cool_dbl(void) {
  return get_temperature_cool() / 100.00;
}

string supla_channel_hvac_value::get_temperature_cool_str(void) {
  char result[20] = {};
  snprintf(result, sizeof(result), "%.2f", get_temperature_cool_dbl());
  return result;
}

short supla_channel_hvac_value::get_temperature(void) {
  if (get_mode() == SUPLA_HVAC_MODE_COOL) {
    return get_temperature_cool();
  } else {
    return get_temperature_heat();
  }
}

double supla_channel_hvac_value::get_temperature_dbl(void) {
  return get_temperature() / 100.00;
}

string supla_channel_hvac_value::get_temperature_str(void) {
  char result[20] = {};
  snprintf(result, sizeof(result), "%.2f", get_temperature_dbl());
  return result;
}

string supla_channel_hvac_value::get_home_assistant_mode(void) {
  if (get_flags() & SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE) {
    return "auto";
  }

  switch (get_mode()) {
    case SUPLA_HVAC_MODE_HEAT:
      return "heat";
    case SUPLA_HVAC_MODE_COOL:
      return "cool";
    case SUPLA_HVAC_MODE_AUTO:
      return "heat_cool";
  }

  return "off";
}

std::string supla_channel_hvac_value::get_home_assistant_action(void) {
  unsigned short flags = get_flags();
  unsigned char mode = get_mode();

  if (flags & SUPLA_HVAC_VALUE_FLAG_HEATING) {
    return "heating";
  } else if (flags & SUPLA_HVAC_VALUE_FLAG_COOLING) {
    return "cooling";
  } else if (mode == SUPLA_HVAC_MODE_OFF || mode == SUPLA_HVAC_MODE_NOT_SET) {
    return "off";
  }

  return "idle";
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
  return ((THVACValue*)raw_value)->IsOn > 0;
}

bool supla_channel_hvac_value::is_heating(void) {
  return ((THVACValue*)raw_value)->IsOn > 0 &&
         (((THVACValue*)raw_value)->Flags & SUPLA_HVAC_VALUE_FLAG_HEATING);
}

bool supla_channel_hvac_value::is_cooling(void) {
  return ((THVACValue*)raw_value)->IsOn > 0 &&
         (((THVACValue*)raw_value)->Flags & SUPLA_HVAC_VALUE_FLAG_COOLING);
}

void supla_channel_hvac_value::turn_on(void) {
  set_mode(SUPLA_HVAC_MODE_CMD_TURN_ON);
}

void supla_channel_hvac_value::turn_off(void) { set_mode(SUPLA_HVAC_MODE_OFF); }

void supla_channel_hvac_value::toggle(void) {
  switch (get_mode()) {
    case SUPLA_HVAC_MODE_HEAT:
    case SUPLA_HVAC_MODE_COOL:
    case SUPLA_HVAC_MODE_AUTO:
    case SUPLA_HVAC_MODE_FAN_ONLY:
    case SUPLA_HVAC_MODE_DRY:
    case SUPLA_HVAC_MODE_CMD_TURN_ON:
      turn_off();
      break;
    default:
      turn_on();
      break;
  }
}

void supla_channel_hvac_value::switch_to_manual(void) {
  set_mode(SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL);
}

void supla_channel_hvac_value::switch_to_program(void) {
  set_mode(SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE);
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
