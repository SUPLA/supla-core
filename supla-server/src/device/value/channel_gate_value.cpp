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

#include "channel_gate_value.h"

#include <string.h>

#include <memory>

#include "device/device.h"

using std::shared_ptr;

supla_channel_gate_value::supla_channel_gate_value(void)
    : supla_channel_value() {
  opening_sensor_level = gsl_unknown;
  partial_opening_sensor_level = gsl_unknown;
}

supla_channel_gate_value::supla_channel_gate_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {
  opening_sensor_level = gsl_unknown;
  partial_opening_sensor_level = gsl_unknown;
}

supla_channel_gate_value::supla_channel_gate_value(
    _gate_sensor_level_enum opening_sensor_level,
    _gate_sensor_level_enum partial_opening_sensor_level)
    : supla_channel_value() {
  this->opening_sensor_level = opening_sensor_level;
  this->partial_opening_sensor_level = partial_opening_sensor_level;
}

_gate_sensor_level_enum supla_channel_gate_value::get_opening_sensor_level(
    void) {
  return opening_sensor_level;
}

void supla_channel_gate_value::set_opening_sensor_level(
    _gate_sensor_level_enum level) {
  opening_sensor_level = level;
}

_gate_sensor_level_enum
supla_channel_gate_value::get_partial_opening_sensor_level(void) {
  return partial_opening_sensor_level;
}

void supla_channel_gate_value::set_partial_opening_sensor_level(
    _gate_sensor_level_enum level) {
  partial_opening_sensor_level = level;
}

_gate_sensor_level_enum supla_channel_gate_value::get_sensor_state(
    supla_user *user, int channel_id) {
  if (user && channel_id) {
    shared_ptr<supla_device> device = user->get_devices()->get(0, channel_id);
    if (device != nullptr) {
      supla_device_channels *channels = device->get_channels();

      if (channels->get_channel_availability_status(channel_id).is_online()) {
        int func = channels->get_channel_func(channel_id);

        switch (func) {
          case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
          case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
            char v = 0;
            if (channels->get_channel_char_value(channel_id, &v)) {
              return v != 0 ? gsl_closed : gsl_open;
            }
            break;
        }
      }
    }
  }

  return gsl_unknown;
}

void supla_channel_gate_value::update_sensors(
    supla_user *user, int opening_sensor_channel_id,
    int partial_opening_sensor_channel_id) {
  opening_sensor_level = get_sensor_state(user, opening_sensor_channel_id);
  partial_opening_sensor_level =
      get_sensor_state(user, partial_opening_sensor_channel_id);
}

bool supla_channel_gate_value::is_relay_hi(void) { return raw_value[0] > 0; }

// static
bool supla_channel_gate_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return true;
  }

  return false;
}
