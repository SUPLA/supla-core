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

using std::map;
using std::string;

#define FIELD_INVERTED_LOGIC 1

const map<unsigned _supla_int16_t, string> binary_sensor_config::field_map = {
    {FIELD_INVERTED_LOGIC, "invertedLogic"}};

binary_sensor_config::binary_sensor_config(void) : supla_json_config() {}

binary_sensor_config::binary_sensor_config(supla_json_config *root)
    : supla_json_config(root) {}

void binary_sensor_config::merge(supla_json_config *_dst) {
  binary_sensor_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           false);
}

void binary_sensor_config::set_config(TChannelConfig_BinarySensor *config) {
  if (!config) {
    return;
  }

  cJSON *root = get_user_root();
  if (!root) {
    return;
  }

  set_item_value(root, field_map.at(FIELD_INVERTED_LOGIC).c_str(),
                 config->InvertedLogic ? cJSON_True : cJSON_False, true,
                 nullptr, nullptr, 0);
}

bool binary_sensor_config::get_config(TChannelConfig_BinarySensor *config) {
  if (!config) {
    return false;
  }

  *config = {};

  cJSON *root = get_user_root();
  if (!root) {
    return false;
  }

  bool bool_value;
  if (get_bool(root, field_map.at(FIELD_INVERTED_LOGIC).c_str(), &bool_value)) {
    config->InvertedLogic = bool_value ? 1 : 0;
    return true;
  }

  return false;
}

bool binary_sensor_config::is_logic_inverted(void) {
  TChannelConfig_BinarySensor config = {};
  if (get_config(&config)) {
    return config.InvertedLogic;
  }
  return false;
}
