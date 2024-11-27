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

#include "channel_fb_value.h"

#include "jsonconfig/channel/facade_blind_config.h"
#include "log.h"

using std::shared_ptr;

supla_channel_fb_value::supla_channel_fb_value(
    const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(raw_value) {}

supla_channel_fb_value::supla_channel_fb_value(
    const TDSC_FacadeBlindValue *value)
    : supla_channel_value() {
  memcpy(raw_value, value, sizeof(TDSC_FacadeBlindValue));
}

const TDSC_FacadeBlindValue *supla_channel_fb_value::get_fb_value(void) {
  return (TDSC_FacadeBlindValue *)raw_value;
}

void supla_channel_fb_value::set_fb_value(TDSC_FacadeBlindValue *value) {
  memset(raw_value, 0, sizeof(raw_value));
  memcpy(raw_value, value, sizeof(TDSC_FacadeBlindValue));
}

char supla_channel_fb_value::get_position(void) {
  return ((TDSC_FacadeBlindValue *)raw_value)->position;
}

char supla_channel_fb_value::get_tilt(void) {
  return ((TDSC_FacadeBlindValue *)raw_value)->tilt;
}

double supla_channel_fb_value::get_tilt_angle(supla_json_config *config) {
  double angle0 = 0;
  double angle100 = 180;

  if (config) {
    facade_blind_config fb_config(config);

    TChannelConfig_FacadeBlind raw_config = {};
    if (fb_config.get_config(&raw_config) &&
        raw_config.Tilt0Angle != raw_config.Tilt100Angle) {
      angle0 = raw_config.Tilt0Angle;
      angle100 = raw_config.Tilt100Angle;
    }
  }

  return angle0 + (angle100 - angle0) / 100.0 * get_tilt();
}

void supla_channel_fb_value::apply_channel_properties(
    int type, unsigned char protocol_version, int param1, int param2,
    int param3, int param4, supla_json_config *json_config) {}

// static
bool supla_channel_fb_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
    case SUPLA_CHANNELFNC_VERTICAL_BLIND:
      return true;
  }

  return false;
}

bool supla_channel_fb_value::get_vbt_value(_vbt_var_name_e var_name,
                                           double *value) {
  *value = get_fb_value()->position;
  return true;
}
