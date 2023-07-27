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

#include "channel_value_factory.h"

#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_dgf_value.h"
#include "device/value/channel_em_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_gate_value.h"
#include "device/value/channel_hvac_value.h"
#include "device/value/channel_ic_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_openclosed_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_thermostat_value.h"
#include "device/value/channel_valve_value.h"

supla_channel_value_factory::supla_channel_value_factory(void) {}

// static
supla_channel_value *supla_channel_value_factory::new_value(
    char value[SUPLA_CHANNELVALUE_SIZE], int type, int func, supla_user *user,
    int param2, int param3) {
  if (!func) {
    return new supla_channel_value(value);
  }

  if (supla_channel_rs_value::is_function_supported(func)) {
    supla_channel_rs_value *rs_value = new supla_channel_rs_value(value);
    rs_value->update_sensor(user, param2);
    return rs_value;
  }

  if (supla_channel_gate_value::is_function_supported(func)) {
    supla_channel_gate_value *gate_value = new supla_channel_gate_value(value);

    gate_value->update_sensors(user, param2, param3);
    return gate_value;
  }

  if (supla_channel_onoff_value::is_function_supported(func)) {
    return new supla_channel_onoff_value(value);
  }

  if (supla_channel_rgbw_value::is_function_supported(func)) {
    return new supla_channel_rgbw_value(value);
  }

  if (supla_channel_valve_value::is_function_supported(func)) {
    return new supla_channel_valve_value(value);
  }

  if (supla_channel_temphum_value::is_function_supported(func)) {
    return new supla_channel_temphum_value(type, func, value);
  }

  if (supla_channel_binary_sensor_value::is_function_supported(func)) {
    return new supla_channel_binary_sensor_value(value);
  }

  if (supla_channel_floating_point_sensor_value::is_function_supported(func)) {
    return new supla_channel_floating_point_sensor_value(value);
  }

  if (supla_channel_openclosed_value::is_function_supported(func)) {
    return new supla_channel_openclosed_value(value);
  }

  if (supla_channel_thermostat_value::is_function_supported(func)) {
    return new supla_channel_thermostat_value(value);
  }

  if (supla_channel_dgf_value::is_function_supported(func)) {
    return new supla_channel_dgf_value(value);
  }

  if (supla_channel_ic_value::is_function_supported(func)) {
    return new supla_channel_ic_value(value);
  }

  if (supla_channel_em_value::is_function_supported(func)) {
    return new supla_channel_em_value(value);
  }

  if (supla_channel_hvac_value::is_function_supported(func)) {
    return new supla_channel_hvac_value(value);
  }

  return new supla_channel_value(value);
}
