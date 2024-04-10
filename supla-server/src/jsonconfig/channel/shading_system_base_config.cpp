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

#include "shading_system_base_config.h"

using std::map;
using std::string;

#define FIELD_CLOSING_TIME_MS 1
#define FIELD_OPENING_TIME_MS 2
#define FIELD_MOTOR_UPSIDE_DOWN 3
#define FIELD_BUTTONS_UPSIDE_DOWN 4
#define FIELD_TIME_MARGIN 5

const map<unsigned _supla_int16_t, string>
    shading_system_base_config::field_map = {
        {FIELD_CLOSING_TIME_MS, "closingTimeMS"},
        {FIELD_OPENING_TIME_MS, "openingTimeMS"},
        {FIELD_MOTOR_UPSIDE_DOWN, "motorUpsideDown"},
        {FIELD_BUTTONS_UPSIDE_DOWN, "buttonsUpsideDown"},
        {FIELD_TIME_MARGIN, "timeMargin"}};

shading_system_base_config::shading_system_base_config(void)
    : supla_json_config() {}

shading_system_base_config::shading_system_base_config(supla_json_config *root)
    : supla_json_config(root) {}

map<unsigned _supla_int16_t, string> shading_system_base_config::get_field_map(
    void) {
  return field_map;
}

void shading_system_base_config::set_config(_supla_int_t closing_time_ms,
                                            _supla_int_t opening_time_ms,
                                            unsigned char motor_upside_down,
                                            unsigned char buttons_upside_down,
                                            signed char time_margin) {
  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  set_item_value(user_root, field_map.at(FIELD_CLOSING_TIME_MS).c_str(),
                 cJSON_Number, true, nullptr, nullptr, closing_time_ms);

  set_item_value(user_root, field_map.at(FIELD_OPENING_TIME_MS).c_str(),
                 cJSON_Number, true, nullptr, nullptr, opening_time_ms);

  set_item_value(user_root, field_map.at(FIELD_MOTOR_UPSIDE_DOWN).c_str(),
                 motor_upside_down ? cJSON_True : cJSON_False, true, nullptr,
                 nullptr, 0);

  set_item_value(user_root, field_map.at(FIELD_BUTTONS_UPSIDE_DOWN).c_str(),
                 buttons_upside_down ? cJSON_True : cJSON_False, true, nullptr,
                 nullptr, 0);

  if (time_margin >= -1 && time_margin <= 100) {
    set_item_value(user_root, field_map.at(FIELD_TIME_MARGIN).c_str(),
                   cJSON_Number, true, nullptr, nullptr, time_margin);
  }
}

bool shading_system_base_config::get_config(_supla_int_t *closing_time_ms,
                                            _supla_int_t *opening_time_ms,
                                            unsigned char *motor_upside_down,
                                            unsigned char *buttons_upside_down,
                                            signed char *time_margin) {
  bool result = false;

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  double dbl_value = 0;

  if (closing_time_ms) {
    if (get_double(user_root, field_map.at(FIELD_CLOSING_TIME_MS).c_str(),
                   &dbl_value)) {
      *closing_time_ms = dbl_value;
      result = true;
    } else {
      *closing_time_ms = 0;
    }
  }

  if (opening_time_ms) {
    if (get_double(user_root, field_map.at(FIELD_OPENING_TIME_MS).c_str(),
                   &dbl_value)) {
      *opening_time_ms = dbl_value;
      result = true;
    } else {
      *opening_time_ms = 0;
    }
  }

  bool bool_value = false;

  if (motor_upside_down) {
    if (get_bool(user_root, field_map.at(FIELD_MOTOR_UPSIDE_DOWN).c_str(),
                 &bool_value)) {
      *motor_upside_down = bool_value ? 1 : 0;
      result = true;
    } else {
      *motor_upside_down = false;
    }
  }

  if (buttons_upside_down) {
    if (get_bool(user_root, field_map.at(FIELD_BUTTONS_UPSIDE_DOWN).c_str(),
                 &bool_value)) {
      *buttons_upside_down = bool_value ? 1 : 0;
      result = true;
    } else {
      *buttons_upside_down = false;
    }
  }

  if (time_margin) {
    if (get_double(user_root, field_map.at(FIELD_TIME_MARGIN).c_str(),
                   &dbl_value) &&
        dbl_value >= -1 && dbl_value <= 100) {
      *time_margin = dbl_value;
      result = true;
    } else {
      *time_margin = -1;
    }
  }

  return result;
}
