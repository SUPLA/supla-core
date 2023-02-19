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

#include "channel_valve_value.h"

#include <string.h>

supla_channel_valve_value::supla_channel_valve_value()
    : supla_channel_value() {}

supla_channel_valve_value::supla_channel_valve_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

supla_channel_valve_value::supla_channel_valve_value(
    const TValve_Value *value) {
  memcpy(raw_value, value, sizeof(TValve_Value));
}

void supla_channel_valve_value::get_valve_value(TValve_Value *value) {
  memcpy(value, raw_value, sizeof(TValve_Value));
}

const TValve_Value *supla_channel_valve_value::get_valve_value(void) {
  return (TValve_Value *)raw_value;
}

void supla_channel_valve_value::set_valve_value(TValve_Value *value) {
  memset(raw_value, 0, sizeof(raw_value));
  memcpy(raw_value, value, sizeof(TValve_Value));
}
