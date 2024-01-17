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

#include "general_purpose_base_config.h"

using std::map;
using std::string;

#define FIELD_VALUE_DIVIDER 1
#define FIELD_VALUE_MULTIPLIER 2
#define FIELD_VALUE_ADDED 3
#define FIELD_VALUE_PRECISION 4
#define FIELD_UNIT_BEFORE_VALUE 5
#define FIELD_UNIT_AFTER_VALUE 6
#define FIELD_NO_SPACE_AFTER_VALUE 7
#define FIELD_KEEP_HISTORY 8

#define FIELD_DEFAULT_VALUE_DIVIDER 9
#define FIELD_DEFAULT_VALUE_ADDED 10
#define FIELD_DEFAULT_VALUE_MULTIPLIER 11
#define FIELD_DEFAULT_VALUE_PRECISION 12
#define FIELD_DEFAULT_UNIT_BEFORE_VALUE 13
#define FIELD_DEFAULT_UNIT_AFTER_VALUE 14

const map<unsigned _supla_int16_t, string>
    general_purpose_base_config::field_map = {
        {FIELD_VALUE_DIVIDER, "valueDivider"},
        {FIELD_VALUE_MULTIPLIER, "valueMultiplier"},
        {FIELD_VALUE_ADDED, "valueAdded"},
        {FIELD_VALUE_PRECISION, "valuePrecision"},
        {FIELD_UNIT_BEFORE_VALUE, "unitBeforeValue"},
        {FIELD_UNIT_AFTER_VALUE, "unitAfterValue"},
        {FIELD_NO_SPACE_AFTER_VALUE, "noSpaceAfterValue"},
        {FIELD_KEEP_HISTORY, "keepHistory"},
        {FIELD_DEFAULT_VALUE_DIVIDER, "defaultValueDivider"},
        {FIELD_DEFAULT_VALUE_MULTIPLIER, "defaultValueMultiplier"},
        {FIELD_DEFAULT_VALUE_ADDED, "defaultValueAdded"},
        {FIELD_DEFAULT_VALUE_PRECISION, "defaultValuePrecision"},
        {FIELD_DEFAULT_UNIT_BEFORE_VALUE, "defaultUnitBeforeValue"},
        {FIELD_DEFAULT_UNIT_AFTER_VALUE, "defaultUnitAfterValue"}};

general_purpose_base_config::general_purpose_base_config(
    supla_json_config *root)
    : supla_json_config(root) {}

general_purpose_base_config::general_purpose_base_config(void)
    : supla_json_config() {}

map<unsigned _supla_int16_t, string> general_purpose_base_config::get_field_map(
    void) {
  return field_map;
}

void general_purpose_base_config::set_config(
    _supla_int_t value_divider, _supla_int_t value_multiplier,
    _supla_int64_t value_added, unsigned char value_precision,
    const std::string &unit_before_value, const std::string &unit_after_value,
    unsigned char no_space_after_value, unsigned char keep_history,
    _supla_int_t default_value_divider, _supla_int_t default_value_multiplier,
    _supla_int64_t default_value_added, unsigned char default_value_precision,
    const std::string &default_unit_before_value,
    const std::string &default_unit_after_value) {
  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return;
  }

  set_item_value(user_root, field_map.at(FIELD_VALUE_DIVIDER).c_str(),
                 cJSON_Number, true, nullptr, nullptr, value_divider);

  set_item_value(user_root, field_map.at(FIELD_VALUE_MULTIPLIER).c_str(),
                 cJSON_Number, true, nullptr, nullptr, value_multiplier);

  set_item_value(user_root, field_map.at(FIELD_VALUE_ADDED).c_str(),
                 cJSON_Number, true, nullptr, nullptr, value_added);

  if (value_precision >= 0 && value_precision <= 4) {
    set_item_value(user_root, field_map.at(FIELD_VALUE_PRECISION).c_str(),
                   cJSON_Number, true, nullptr, nullptr, value_precision);
  }

  set_item_value(user_root, field_map.at(FIELD_UNIT_BEFORE_VALUE).c_str(),
                 cJSON_String, true, nullptr, unit_before_value.c_str(), 0);

  set_item_value(user_root, field_map.at(FIELD_UNIT_AFTER_VALUE).c_str(),
                 cJSON_String, true, nullptr, unit_after_value.c_str(), 0);

  set_item_value(user_root, field_map.at(FIELD_NO_SPACE_AFTER_VALUE).c_str(),
                 no_space_after_value ? cJSON_True : cJSON_False, true, nullptr,
                 nullptr, 0);

  set_item_value(user_root, field_map.at(FIELD_KEEP_HISTORY).c_str(),
                 keep_history ? cJSON_True : cJSON_False, true, nullptr,
                 nullptr, 0);

  set_item_value(properties_root,
                 field_map.at(FIELD_DEFAULT_VALUE_DIVIDER).c_str(),
                 cJSON_Number, true, nullptr, nullptr, default_value_divider);

  set_item_value(
      properties_root, field_map.at(FIELD_DEFAULT_VALUE_MULTIPLIER).c_str(),
      cJSON_Number, true, nullptr, nullptr, default_value_multiplier);

  set_item_value(properties_root,
                 field_map.at(FIELD_DEFAULT_VALUE_ADDED).c_str(), cJSON_Number,
                 true, nullptr, nullptr, default_value_added);

  if (default_value_precision >= 0 && default_value_precision <= 4) {
    set_item_value(
        properties_root, field_map.at(FIELD_DEFAULT_VALUE_PRECISION).c_str(),
        cJSON_Number, true, nullptr, nullptr, default_value_precision);
  }

  set_item_value(
      properties_root, field_map.at(FIELD_DEFAULT_UNIT_BEFORE_VALUE).c_str(),
      cJSON_String, true, nullptr, default_unit_before_value.c_str(), 0);

  set_item_value(
      properties_root, field_map.at(FIELD_DEFAULT_UNIT_AFTER_VALUE).c_str(),
      cJSON_String, true, nullptr, default_unit_after_value.c_str(), 0);
}

bool general_purpose_base_config::get_config(
    _supla_int_t *value_divider, _supla_int_t *value_multiplier,
    _supla_int64_t *value_added, unsigned char *value_precision,
    std::string *unit_before_value, std::string *unit_after_value,
    unsigned char *no_space_after_value, unsigned char *keep_history,
    _supla_int_t *default_value_divider, _supla_int_t *default_value_multiplier,
    _supla_int64_t *default_value_added, unsigned char *default_value_precision,
    std::string *default_unit_before_value,
    std::string *default_unit_after_value) {
  bool result = false;

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return result;
  }

  double dbl_value = 0;

  if (value_divider) {
    if (get_double(user_root, field_map.at(FIELD_VALUE_DIVIDER).c_str(),
                   &dbl_value)) {
      *value_divider = dbl_value;
      result = true;
    } else {
      *value_divider = 0;
    }
  }

  if (value_multiplier) {
    if (get_double(user_root, field_map.at(FIELD_VALUE_MULTIPLIER).c_str(),
                   &dbl_value)) {
      *value_multiplier = dbl_value;
      result = true;
    } else {
      *value_multiplier = 0;
    }
  }

  if (value_added) {
    if (get_double(user_root, field_map.at(FIELD_VALUE_ADDED).c_str(),
                   &dbl_value)) {
      *value_added = dbl_value;
      result = true;
    } else {
      *value_added = 0;
    }
  }

  if (value_precision) {
    if (get_double(user_root, field_map.at(FIELD_VALUE_PRECISION).c_str(),
                   &dbl_value) &&
        dbl_value >= 0 && dbl_value <= 10) {
      *value_precision = dbl_value;
      result = true;
    } else {
      *value_precision = 0;
    }
  }

  string str_value;

  if (unit_before_value) {
    if (get_string(user_root, field_map.at(FIELD_UNIT_BEFORE_VALUE).c_str(),
                   &str_value)) {
      *unit_before_value = str_value;
      result = true;
    } else {
      unit_before_value->clear();
    }
  }

  if (unit_after_value) {
    if (get_string(user_root, field_map.at(FIELD_UNIT_AFTER_VALUE).c_str(),
                   &str_value)) {
      *unit_after_value = str_value;
      result = true;
    } else {
      unit_after_value->clear();
    }
  }

  bool bool_value = false;

  if (no_space_after_value) {
    if (get_bool(user_root, field_map.at(FIELD_NO_SPACE_AFTER_VALUE).c_str(),
                 &bool_value)) {
      *no_space_after_value = bool_value ? 1 : 0;
      result = true;
    }
  }

  if (keep_history) {
    if (get_bool(user_root, field_map.at(FIELD_KEEP_HISTORY).c_str(),
                 &bool_value)) {
      *keep_history = bool_value ? 1 : 0;
      result = true;
    }
  }

  if (default_value_divider) {
    if (get_double(properties_root,
                   field_map.at(FIELD_DEFAULT_VALUE_DIVIDER).c_str(),
                   &dbl_value)) {
      *default_value_divider = dbl_value;
      result = true;
    } else {
      *default_value_divider = 0;
    }
  }

  if (default_value_multiplier) {
    if (get_double(properties_root,
                   field_map.at(FIELD_DEFAULT_VALUE_MULTIPLIER).c_str(),
                   &dbl_value)) {
      *default_value_multiplier = dbl_value;
      result = true;
    } else {
      *default_value_multiplier = 0;
    }
  }

  if (default_value_added) {
    if (get_double(properties_root,
                   field_map.at(FIELD_DEFAULT_VALUE_ADDED).c_str(),
                   &dbl_value)) {
      *default_value_added = dbl_value;
      result = true;
    } else {
      *default_value_added = 0;
    }
  }

  if (default_value_precision) {
    if (get_double(properties_root,
                   field_map.at(FIELD_DEFAULT_VALUE_PRECISION).c_str(),
                   &dbl_value) &&
        dbl_value >= 0 && dbl_value <= 10) {
      *default_value_precision = dbl_value;
      result = true;
    } else {
      *default_value_precision = 0;
    }
  }

  if (default_unit_before_value) {
    if (get_string(properties_root,
                   field_map.at(FIELD_DEFAULT_UNIT_BEFORE_VALUE).c_str(),
                   &str_value)) {
      *default_unit_before_value = str_value;
      result = true;
    } else {
      default_unit_before_value->clear();
    }
  }

  if (default_unit_after_value) {
    if (get_string(properties_root,
                   field_map.at(FIELD_DEFAULT_UNIT_AFTER_VALUE).c_str(),
                   &str_value)) {
      *default_unit_after_value = str_value;
      result = true;
    } else {
      default_unit_after_value->clear();
    }
  }

  return result;
}

double general_purpose_base_config::get_value_divider(void) {
  double dbl_value = 0;
  get_double(get_user_root(), field_map.at(FIELD_VALUE_DIVIDER).c_str(),
             &dbl_value);
  return dbl_value * 0.001;
}

double general_purpose_base_config::get_value_multiplier(void) {
  double dbl_value = 0;
  get_double(get_user_root(), field_map.at(FIELD_VALUE_MULTIPLIER).c_str(),
             &dbl_value);
  return dbl_value * 0.001;
}

double general_purpose_base_config::get_value_added(void) {
  double dbl_value = 0;
  get_double(get_user_root(), field_map.at(FIELD_VALUE_ADDED).c_str(),
             &dbl_value);
  return dbl_value * 0.001;
}
