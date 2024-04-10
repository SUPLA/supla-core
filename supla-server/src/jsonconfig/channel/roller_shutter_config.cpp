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

#include "roller_shutter_config.h"

#include <string>

using std::map;
using std::string;

roller_shutter_config::roller_shutter_config(void)
    : shading_system_base_config() {}

roller_shutter_config::roller_shutter_config(supla_json_config *root)
    : shading_system_base_config(root) {}

void roller_shutter_config::merge(supla_json_config *_dst) {
  roller_shutter_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(),
                           get_field_map(), false);
}

void roller_shutter_config::set_config(TChannelConfig_Rollershutter *config) {
  if (config) {
    shading_system_base_config::set_config(
        config->ClosingTimeMS, config->OpeningTimeMS, config->MotorUpsideDown,
        config->ButtonsUpsideDown, config->TimeMargin);
  }
}

bool roller_shutter_config::get_config(TChannelConfig_Rollershutter *config) {
  if (config) {
    return shading_system_base_config::get_config(
        &config->ClosingTimeMS, &config->OpeningTimeMS,
        &config->MotorUpsideDown, &config->ButtonsUpsideDown,
        &config->TimeMargin);
  }

  return false;
}
