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

#include "channel_container_value.h"

#include <string.h>

supla_channel_container_value::supla_channel_container_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {}

supla_channel_container_value::supla_channel_container_value(
    const TContainerChannel_Value *value)
    : supla_abstract_channel_value() {
  memcpy(raw_value, value, sizeof(TContainerChannel_Value));
}

supla_abstract_channel_value *supla_channel_container_value::copy(  // NOLINT
    void) const {                                                   // NOLINT
  return new supla_channel_container_value(raw_value);
}

bool supla_channel_container_value::get_level(unsigned char *level) {
  if (level) {
    *level = get_raw_level();
    if (*level) {
      (*level)--;
      return true;
    }
  }

  return false;
}

unsigned char supla_channel_container_value::get_raw_level(void) {
  if (((TContainerChannel_Value *)raw_value)->level >= 1 &&
      ((TContainerChannel_Value *)raw_value)->level <= 101) {
    return ((TContainerChannel_Value *)raw_value)->level;
  }
  return 0;
}

bool supla_channel_container_value::is_invalid(void) {
  return ((TContainerChannel_Value *)raw_value)->level < 1 ||
         ((TContainerChannel_Value *)raw_value)->level > 101;
}

bool supla_channel_container_value::is_warning_flag_set(void) {
  return ((TContainerChannel_Value *)raw_value)->flags &
         CONTAINER_FLAG_WARNING_LEVEL;
}

bool supla_channel_container_value::is_alarm_flag_set(void) {
  return ((TContainerChannel_Value *)raw_value)->flags &
         CONTAINER_FLAG_ALARM_LEVEL;
}

bool supla_channel_container_value::is_invalid_sensor_state_flag_set(void) {
  return ((TContainerChannel_Value *)raw_value)->flags &
         CONTAINER_FLAG_INVALID_SENSOR_STATE;
}

bool supla_channel_container_value::is_sound_alarm_on(void) {
  return ((TContainerChannel_Value *)raw_value)->flags &
         CONTAINER_FLAG_SOUND_ALARM_ON;
}

unsigned _supla_int16_t supla_channel_container_value::get_raw_flags(void) {
  return ((TContainerChannel_Value *)raw_value)->flags;
}

bool supla_channel_container_value::get_vbt_value(_vbt_var_name_e var_name,
                                                  double *value) {
  switch (var_name) {
    case var_name_invalid_value:
      *value = is_invalid() ? 1 : 0;
      return true;
    case var_name_alarm:
      *value = is_alarm_flag_set() ? 1 : 0;
      return true;
    case var_name_sound_alarm_on:
      *value = is_sound_alarm_on() ? 1 : 0;
      return true;
    case var_name_warning:
      *value = is_warning_flag_set() ? 1 : 0;
      return true;
    case var_name_invalid_sensor_state:
      *value = is_invalid_sensor_state_flag_set() ? 1 : 0;
      return true;
    default: {
      unsigned char level = 0;
      if (get_level(&level)) {
        *value = level;
        return true;
      }
    }

    break;
  }
  return false;
}

// static
bool supla_channel_container_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_CONTAINER:
    case SUPLA_CHANNELFNC_SEPTIC_TANK:
    case SUPLA_CHANNELFNC_WATER_TANK:
      return true;
  }
  return false;
}
