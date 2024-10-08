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

#include "light_switch_config.h"

light_switch_config::light_switch_config(void) : power_switch_config() {}

light_switch_config::light_switch_config(supla_json_config *root)
    : power_switch_config(root) {}

light_switch_config::~light_switch_config(void) {}

void light_switch_config::set_config(
    TChannelConfig_LightSwitch *config,
    supla_abstract_common_channel_properties *props) {
  if (config) {
    TChannelConfig_PowerSwitch ps = {};
    ps.OvercurrentThreshold = config->OvercurrentThreshold;
    ps.OvercurrentMaxAllowed = config->OvercurrentMaxAllowed;
    ps.DefaultRelatedMeterIsSet = config->DefaultRelatedMeterIsSet;
    ps.DefaultRelatedMeterChannelNo = config->DefaultRelatedMeterChannelNo;
    power_switch_config::set_config(&ps, props);
  }
}

bool light_switch_config::get_config(TChannelConfig_LightSwitch *config) {
  if (config) {
    TChannelConfig_PowerSwitch ps = {};
    if (power_switch_config::get_config(&ps)) {
      config->OvercurrentThreshold = ps.OvercurrentThreshold;
      config->OvercurrentMaxAllowed = ps.OvercurrentMaxAllowed;
      config->DefaultRelatedMeterIsSet = ps.DefaultRelatedMeterIsSet;
      config->DefaultRelatedMeterChannelNo = ps.DefaultRelatedMeterChannelNo;
      return true;
    }
  }

  return false;
}
