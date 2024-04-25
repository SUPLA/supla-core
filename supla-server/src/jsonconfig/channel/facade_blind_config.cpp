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

#include "facade_blind_config.h"

using std::map;
using std::string;

#define FIELD_TILTING_TIME_MS 100
#define FIELD_TILT0_ANGLE 101
#define FIELD_TILT100_ANGLE 102
#define FIELD_TILT_CONTROL_TYPE 103

const map<unsigned _supla_int16_t, string> facade_blind_config::field_map = {
    {FIELD_TILTING_TIME_MS, "tiltingTimeMs"},
    {FIELD_TILT0_ANGLE, "tilt0Angle"},
    {FIELD_TILT100_ANGLE, "tilt100Angle"},
    {FIELD_TILT_CONTROL_TYPE, "tiltControlType"}};

facade_blind_config::facade_blind_config(void) : roller_shutter_config() {}

facade_blind_config::facade_blind_config(supla_json_config *root)
    : roller_shutter_config(root) {}

void facade_blind_config::merge(supla_json_config *_dst) {
  map<unsigned _supla_int16_t, string> fmap = get_field_map();
  fmap.insert(field_map.begin(), field_map.end());

  facade_blind_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), fmap, false);
}

string facade_blind_config::type_to_string(unsigned char type) {
  switch (type) {
    case SUPLA_TILT_CONTROL_TYPE_STANDS_IN_POSITION_WHILE_TILTING:
      return "STANDS_IN_POSITION_WHILE_TILTING";
    case SUPLA_TILT_CONTROL_TYPE_CHANGES_POSITION_WHILE_TILTING:
      return "CHANGES_POSITION_WHILE_TILTING";
    case SUPLA_TILT_CONTROL_TYPE_TILTS_ONLY_WHEN_FULLY_CLOSED:
      return "TILTS_ONLY_WHEN_FULLY_CLOSED";
  }

  return "UNKNOWN";
}

unsigned char facade_blind_config::string_to_type(const string &type) {
  if (type == "STANDS_IN_POSITION_WHILE_TILTING") {
    return SUPLA_TILT_CONTROL_TYPE_STANDS_IN_POSITION_WHILE_TILTING;
  } else if (type == "CHANGES_POSITION_WHILE_TILTING") {
    return SUPLA_TILT_CONTROL_TYPE_CHANGES_POSITION_WHILE_TILTING;
  } else if (type == "TILTS_ONLY_WHEN_FULLY_CLOSED") {
    return SUPLA_TILT_CONTROL_TYPE_TILTS_ONLY_WHEN_FULLY_CLOSED;
  }

  return SUPLA_TILT_CONTROL_TYPE_UNKNOWN;
}

void facade_blind_config::set_config(TChannelConfig_FacadeBlind *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  roller_shutter_config::set_config(
      config->ClosingTimeMS, config->OpeningTimeMS, config->MotorUpsideDown,
      config->ButtonsUpsideDown, config->TimeMargin);

  set_item_value(user_root, field_map.at(FIELD_TILTING_TIME_MS).c_str(),
                 cJSON_Number, true, nullptr, nullptr, config->TiltingTimeMS);

  if (config->Tilt0Angle >= 0 && config->Tilt0Angle <= 180) {
    set_item_value(user_root, field_map.at(FIELD_TILT0_ANGLE).c_str(),
                   cJSON_Number, true, nullptr, nullptr, config->Tilt0Angle);
  }

  if (config->Tilt100Angle >= 0 && config->Tilt100Angle <= 180) {
    set_item_value(user_root, field_map.at(FIELD_TILT100_ANGLE).c_str(),
                   cJSON_Number, true, nullptr, nullptr, config->Tilt100Angle);
  }

  set_item_value(user_root, field_map.at(FIELD_TILT_CONTROL_TYPE).c_str(),
                 cJSON_String, true, nullptr,
                 type_to_string(config->TiltControlType).c_str(), 0);
}

bool facade_blind_config::get_config(TChannelConfig_FacadeBlind *config) {
  bool result = false;

  if (!config) {
    return result;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  if (roller_shutter_config::get_config(
          &config->ClosingTimeMS, &config->OpeningTimeMS,
          &config->MotorUpsideDown, &config->ButtonsUpsideDown,
          &config->TimeMargin)) {
    result = true;
  }

  double dbl_value = 0;

  if (get_double(user_root, field_map.at(FIELD_TILTING_TIME_MS).c_str(),
                 &dbl_value)) {
    config->TiltingTimeMS = dbl_value;
    result = true;
  } else {
    config->TiltingTimeMS = 0;
  }

  if (get_double(user_root, field_map.at(FIELD_TILT0_ANGLE).c_str(),
                 &dbl_value) &&
      dbl_value >= 0 && dbl_value <= 360) {
    config->Tilt0Angle = dbl_value;
    result = true;
  } else {
    config->Tilt0Angle = 0;
  }

  if (get_double(user_root, field_map.at(FIELD_TILT100_ANGLE).c_str(),
                 &dbl_value) &&
      dbl_value >= 0 && dbl_value <= 360) {
    config->Tilt100Angle = dbl_value;
    result = true;
  } else {
    config->Tilt100Angle = 0;
  }

  std::string str_value;
  if (get_string(user_root, field_map.at(FIELD_TILT_CONTROL_TYPE).c_str(),
                 &str_value)) {
    config->TiltControlType = string_to_type(str_value);
    result = true;
  }

  return result;
}
