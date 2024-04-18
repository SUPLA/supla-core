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

using std::map;
using std::string;

#define FIELD_CLOSING_TIME_MS 1
#define FIELD_OPENING_TIME_MS 2
#define FIELD_MOTOR_UPSIDE_DOWN 3
#define FIELD_BUTTONS_UPSIDE_DOWN 4
#define FIELD_TIME_MARGIN 5

const map<unsigned _supla_int16_t, string> roller_shutter_config::field_map = {
    {FIELD_CLOSING_TIME_MS, "closingTimeMs"},
    {FIELD_OPENING_TIME_MS, "openingTimeMs"},
    {FIELD_MOTOR_UPSIDE_DOWN, "motorUpsideDown"},
    {FIELD_BUTTONS_UPSIDE_DOWN, "buttonsUpsideDown"},
    {FIELD_TIME_MARGIN, "timeMargin"}};

roller_shutter_config::roller_shutter_config(void) : supla_json_config() {}

roller_shutter_config::roller_shutter_config(supla_json_config *root)
    : supla_json_config(root) {}

void roller_shutter_config::merge(supla_json_config *_dst) {
  roller_shutter_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           false);
}

map<unsigned _supla_int16_t, string> roller_shutter_config::get_field_map(
    void) {
  return field_map;
}

string roller_shutter_config::time_margin_to_string(char time_margin) {
  if (time_margin == -1) {
    return "DEVICE_SPECIFIC";
  }

  return "";
}

char roller_shutter_config::string_to_time_margin(const string &tm) {
  if (tm == "DEVICE_SPECIFIC") {
    return -1;
  }

  return 0;
}

void roller_shutter_config::set_config(_supla_int_t closing_time_ms,
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

  if (motor_upside_down) {
    set_item_value(user_root, field_map.at(FIELD_MOTOR_UPSIDE_DOWN).c_str(),
                   motor_upside_down == 2 ? cJSON_True : cJSON_False, true,
                   nullptr, nullptr, 0);
  }

  if (buttons_upside_down) {
    set_item_value(user_root, field_map.at(FIELD_BUTTONS_UPSIDE_DOWN).c_str(),
                   buttons_upside_down == 2 ? cJSON_True : cJSON_False, true,
                   nullptr, nullptr, 0);
  }

  if (time_margin) {
    if (time_margin == -1) {
      set_item_value(user_root, field_map.at(FIELD_TIME_MARGIN).c_str(),
                     cJSON_String, true, nullptr,
                     time_margin_to_string(time_margin).c_str(), 0);

    } else if (time_margin >= 1 && time_margin <= 101) {
      set_item_value(user_root, field_map.at(FIELD_TIME_MARGIN).c_str(),
                     cJSON_Number, true, nullptr, nullptr, time_margin - 1);
    }
  }
}

bool roller_shutter_config::get_config(_supla_int_t *closing_time_ms,
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
      *motor_upside_down = bool_value ? 2 : 1;
      result = true;
    } else {
      *motor_upside_down = 0;
    }
  }

  if (buttons_upside_down) {
    if (get_bool(user_root, field_map.at(FIELD_BUTTONS_UPSIDE_DOWN).c_str(),
                 &bool_value)) {
      *buttons_upside_down = bool_value ? 2 : 1;
      result = true;
    } else {
      *buttons_upside_down = 0;
    }
  }

  string str_value;

  if (time_margin) {
    if (get_double(user_root, field_map.at(FIELD_TIME_MARGIN).c_str(),
                   &dbl_value) &&
        dbl_value >= 0 && dbl_value <= 100) {
      *time_margin = dbl_value + 1;
      result = true;
    } else if (get_string(user_root, field_map.at(FIELD_TIME_MARGIN).c_str(),
                          &str_value)) {
      *time_margin = string_to_time_margin(str_value);
    } else {
      *time_margin = 0;
    }
  }

  return result;
}

unsigned int roller_shutter_config::get_value_duration(void) {
  unsigned int result = 0;

  _supla_int_t closing_time_ms = 0;
  _supla_int_t opening_time_ms = 0;

  if (get_config(&closing_time_ms, &opening_time_ms, nullptr, nullptr,
                 nullptr)) {
    opening_time_ms /= 100;
    closing_time_ms /= 100;

    result = opening_time_ms & 0xFFFF;
    result <<= 16;
    result |= closing_time_ms & 0xFFFF;
  }

  return result;
}

void roller_shutter_config::set_config(TChannelConfig_Rollershutter *config) {
  if (config) {
    set_config(config->ClosingTimeMS, config->OpeningTimeMS,
               config->MotorUpsideDown, config->ButtonsUpsideDown,
               config->TimeMargin);
  }
}

bool roller_shutter_config::get_config(TChannelConfig_Rollershutter *config) {
  if (config) {
    return get_config(&config->ClosingTimeMS, &config->OpeningTimeMS,
                      &config->MotorUpsideDown, &config->ButtonsUpsideDown,
                      &config->TimeMargin);
  }

  return false;
}
