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

#include "jsonconfig/channel/binary_sensor_config.h"

supla_channel_binary_sensor_value::supla_channel_binary_sensor_value(void)
    : supla_abstract_channel_value() {}

supla_channel_binary_sensor_value::supla_channel_binary_sensor_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {}

supla_channel_binary_sensor_value::supla_channel_binary_sensor_value(bool hi)
    : supla_abstract_channel_value() {
  set_hi(hi);
}

supla_abstract_channel_value *
supla_channel_binary_sensor_value::copy(  // NOLINT
    void) const {                         // NOLINT
  return new supla_channel_binary_sensor_value(raw_value);
}

void supla_channel_binary_sensor_value::set_hi(bool hi) {
  raw_value[0] = hi ? 1 : 0;
}

bool supla_channel_binary_sensor_value::is_hi(void) { return raw_value[0] > 0; }

void supla_channel_binary_sensor_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {
  if (type == SUPLA_CHANNELTYPE_SENSORNC) {
    set_hi(!is_hi());
  }

  if (json_config) {
    binary_sensor_config cfg(json_config);
    if (cfg.is_logic_inverted()) {
      set_hi(!is_hi());
    }
  }
}

// static
bool supla_channel_binary_sensor_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_MAILSENSOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_HOTELCARDSENSOR:
    case SUPLA_CHANNELFNC_ALARMARMAMENTSENSOR:
    case SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR:
    case SUPLA_CHANNELFNC_FLOOD_SENSOR:
      return true;
  }

  return false;
}

bool supla_channel_binary_sensor_value::get_vbt_value(_vbt_var_name_e var_name,
                                                      double *value) {
  *value = is_hi() ? 1.0 : 0.0;
  return true;
}
