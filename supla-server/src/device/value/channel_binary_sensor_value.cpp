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

#include "channel_binary_sensor_value.h"

#include <string.h>

supla_channel_binary_sensor_value::supla_channel_binary_sensor_value(void)
    : supla_channel_value() {}

supla_channel_binary_sensor_value::supla_channel_binary_sensor_value(
    char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

supla_channel_binary_sensor_value::supla_channel_binary_sensor_value(bool hi)
    : supla_channel_value() {
  set_hi(hi);
}

void supla_channel_binary_sensor_value::set_hi(bool hi) {
  raw_value[0] = hi ? 1 : 0;
}

bool supla_channel_binary_sensor_value::is_hi(void) { return raw_value[0] > 0; }

void supla_channel_binary_sensor_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, channel_json_config *json_config,
    _logger_purpose_t *logger_data) {
  if (type == SUPLA_CHANNELTYPE_SENSORNC) {
    set_hi(!is_hi());
  }

  if (param3 == 1) {  // inverted logic
    set_hi(!is_hi());
  }
}
