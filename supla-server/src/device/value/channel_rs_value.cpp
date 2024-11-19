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

#include "channel_rs_value.h"

#include <string.h>

#include <memory>

#include "device/device.h"

using std::shared_ptr;

supla_channel_rs_value::supla_channel_rs_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {
  opening_sensor_level = rsl_unknown;
}

supla_channel_rs_value::supla_channel_rs_value(
    const TDSC_RollerShutterValue *value)
    : supla_channel_value() {
  memcpy(raw_value, value, sizeof(TDSC_RollerShutterValue));
  opening_sensor_level = rsl_unknown;
}

const TDSC_RollerShutterValue *supla_channel_rs_value::get_rs_value(void) {
  return (TDSC_RollerShutterValue *)raw_value;
}

void supla_channel_rs_value::set_rs_value(TDSC_RollerShutterValue *value) {
  memset(raw_value, 0, sizeof(raw_value));
  memcpy(raw_value, value, sizeof(TDSC_RollerShutterValue));
}

_rs_sensor_level_enum supla_channel_rs_value::get_opening_sensor_level(void) {
  return opening_sensor_level;
}

void supla_channel_rs_value::set_opening_sensor_level(
    _rs_sensor_level_enum level) {
  opening_sensor_level = level;
}

void supla_channel_rs_value::update_sensor(supla_user *user,
                                           int opening_sensor_channel_id) {
  opening_sensor_level = rsl_unknown;

  if (user && opening_sensor_channel_id) {
    shared_ptr<supla_device> device =
        user->get_devices()->get(0, opening_sensor_channel_id);
    if (device != nullptr) {
      supla_device_channels *channels = device->get_channels();

      if (channels->is_channel_online(opening_sensor_channel_id)) {
        int func = channels->get_channel_func(opening_sensor_channel_id);

        switch (func) {
          case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
          case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
            char v = 0;
            if (channels->get_channel_char_value(opening_sensor_channel_id,
                                                 &v)) {
              opening_sensor_level = v != 0 ? rsl_closed : rsl_open;
            }
            break;
        }
      }
    }
  }
}

void supla_channel_rs_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {
  ((TDSC_RollerShutterValue *)raw_value)->bottom_position = param4;
}

// static
bool supla_channel_rs_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_TERRACE_AWNING:
    case SUPLA_CHANNELFNC_PROJECTOR_SCREEN:
    case SUPLA_CHANNELFNC_CURTAIN:
    case SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR:
      return true;
  }

  return false;
}

bool supla_channel_rs_value::get_vbt_value(_vbt_var_name_e var_name,
                                           double *value) {
  switch (var_name) {
    case var_name_calibration_failed:
      *value = get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_FAILED ? 1 : 0;
      break;
    case var_name_calibration_lost:
      *value = get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_LOST ? 1 : 0;
      break;
    case var_name_motor_problem:
      *value = get_rs_value()->flags & RS_VALUE_FLAG_MOTOR_PROBLEM ? 1 : 0;
      break;
    case var_name_calibration_in_progress:
      *value =
          get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_IN_PROGRESS ? 1 : 0;
      break;
    case var_name_is_any_error_set:
      *value = ((get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_FAILED) ||
                (get_rs_value()->flags & RS_VALUE_FLAG_CALIBRATION_LOST) ||
                (get_rs_value()->flags & RS_VALUE_FLAG_MOTOR_PROBLEM))
                   ? 1
                   : 0;
      break;
    default:
      *value = get_rs_value()->position;
      break;
  }

  return true;
}
