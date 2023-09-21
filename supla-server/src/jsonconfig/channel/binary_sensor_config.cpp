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

#include "binary_sensor_config.h"

binary_sensor_config::binary_sensor_config(void) : channel_json_config() {}

binary_sensor_config::binary_sensor_config(supla_json_config *root)
    : channel_json_config(root) {}

void binary_sensor_config::merge(supla_json_config *_dst) {}

void binary_sensor_config::set_config(TChannelConfig_BinarySensor *config) {}

bool binary_sensor_config::get_config(TChannelConfig_BinarySensor *config) {}

bool binary_sensor_config::is_logic_inverted(void) {
  TChannelConfig_BinarySensor config = {};
  if (get_config(&config)) {
    return config.InvertedLogic;
  }
  return false;
}
