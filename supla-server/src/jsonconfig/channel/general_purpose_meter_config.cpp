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

#include "general_purpose_meter_config.h"

using std::map;
using std::string;

// Keys cannot overlap with general_purpose_base_config::field_map keys
#define FIELD_METER_CHART_TYPE 1000
#define FIELD_INCLUDE_VALUE_ADDED_IN_HISTORY 1001
#define FIELD_FILL_MISSING_DATA 1002
#define FIELD_COUNTER_TYPE 1003

const map<unsigned _supla_int16_t, string>
    general_purpose_meter_config::field_map = {
        {FIELD_METER_CHART_TYPE, "chartType"},
        {FIELD_INCLUDE_VALUE_ADDED_IN_HISTORY, "includeValueAddedInHistory"},
        {FIELD_FILL_MISSING_DATA, "fillMissingData"},
        {FIELD_COUNTER_TYPE, "counterType"}};

general_purpose_meter_config::general_purpose_meter_config(
    supla_json_config *root)
    : general_purpose_base_config(root) {}

general_purpose_meter_config::general_purpose_meter_config(void)
    : general_purpose_base_config() {}

void general_purpose_meter_config::merge(supla_json_config *_dst) {
  map<unsigned _supla_int16_t, string> fmap = get_field_map();
  fmap.insert(field_map.begin(), field_map.end());

  general_purpose_meter_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), fmap, true);
  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           fmap, true);
}

string general_purpose_meter_config::chart_type_to_string(
    unsigned char chart_type) {
  if (chart_type == SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_LINEAR) {
    return "LINEAR";
  }

  return "BAR";
}

unsigned char general_purpose_meter_config::string_to_chart_type(
    const string &chart_type) {
  if (chart_type == "LINEAR") {
    return SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_LINEAR;
  }

  return SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_BAR;
}

string general_purpose_meter_config::counter_type_to_string(
    unsigned char counter_type) {
  switch (counter_type) {
    case SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_ALWAYS_INCREMENT:
      return "ALWAYS_INCREMENT";
    case SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_ALWAYS_DECREMENT:
      return "ALWAYS_DECREMENT";
  }

  return "INCREMENT_AND_DECREMENT";
}

unsigned char general_purpose_meter_config::string_to_counter_type(
    const string &chart_type) {
  if (chart_type == "ALWAYS_INCREMENT") {
    return SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_ALWAYS_INCREMENT;
  } else if (chart_type == "ALWAYS_DECREMENT") {
    return SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_ALWAYS_DECREMENT;
  }

  return SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_INCREMENT_DECREMENT;
}

void general_purpose_meter_config::set_config(
    TChannelConfig_GeneralPurposeMeter *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  string unit_before_value(config->UnitBeforeValue,
                           sizeof(config->UnitBeforeValue));
  string unit_after_value(config->UnitAfterValue,
                          sizeof(config->UnitAfterValue));
  string default_unit_before_value(config->DefaultUnitBeforeValue,
                                   sizeof(config->DefaultUnitBeforeValue));
  string default_unit_after_value(config->DefaultUnitAfterValue,
                                  sizeof(config->DefaultUnitAfterValue));

  general_purpose_base_config::set_config(
      config->ValueDivider, config->ValueMultiplier, config->ValueAdded,
      config->ValuePrecision, unit_before_value, unit_after_value,
      config->NoSpaceBeforeValue, config->NoSpaceAfterValue,
      config->KeepHistory, config->RefreshIntervalMs,
      config->DefaultValueDivider, config->DefaultValueMultiplier,
      config->DefaultValueAdded, config->DefaultValuePrecision,
      default_unit_before_value, default_unit_after_value);

  set_item_value(user_root, field_map.at(FIELD_METER_CHART_TYPE).c_str(),
                 cJSON_String, true, nullptr,
                 chart_type_to_string(config->ChartType).c_str(), 0);

  set_item_value(user_root,
                 field_map.at(FIELD_INCLUDE_VALUE_ADDED_IN_HISTORY).c_str(),
                 config->IncludeValueAddedInHistory ? cJSON_True : cJSON_False,
                 true, nullptr, nullptr, 0);

  set_item_value(user_root, field_map.at(FIELD_FILL_MISSING_DATA).c_str(),
                 config->FillMissingData ? cJSON_True : cJSON_False, true,
                 nullptr, nullptr, 0);

  set_item_value(user_root, field_map.at(FIELD_COUNTER_TYPE).c_str(),
                 cJSON_String, true, nullptr,
                 counter_type_to_string(config->CounterType).c_str(), 0);
}

bool general_purpose_meter_config::get_config(
    TChannelConfig_GeneralPurposeMeter *config) {
  bool result = false;

  if (!config) {
    return result;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  string unit_before_value, unit_after_value, default_unit_before_value,
      default_unit_after_value;

  // Referring to the TChannelConfig_GeneralPurposeMeter structure via
  // pointers on ARM processors results in application interruption (SIGBUS).
  // Therefore, we use additional intermediate variables.

  _supla_int_t value_divider = 0;
  _supla_int_t value_multiplier = 0;
  _supla_int64_t value_added = 0;
  unsigned char value_precision = 0;
  unsigned char no_space_before_value = 0;
  unsigned char no_space_after_value = 0;
  unsigned char keep_history = 0;
  unsigned _supla_int16_t refresh_interval_ms = 0;
  _supla_int_t default_value_divider = 0;
  _supla_int_t default_value_multiplier = 0;
  _supla_int64_t default_value_added = 0;
  unsigned char default_value_precision = 0;

  result = general_purpose_base_config::get_config(
      &value_divider, &value_multiplier, &value_added, &value_precision,
      &unit_before_value, &unit_after_value, &no_space_before_value,
      &no_space_after_value, &keep_history, &refresh_interval_ms,
      &default_value_divider, &default_value_multiplier, &default_value_added,
      &default_value_precision, &default_unit_before_value,
      &default_unit_after_value);

  if (result) {
    config->ValueDivider = value_divider;
    config->ValueMultiplier = value_multiplier;
    config->ValueAdded = value_added;
    config->ValuePrecision = value_precision;
    config->NoSpaceBeforeValue = no_space_before_value;
    config->NoSpaceAfterValue = no_space_after_value;
    config->KeepHistory = keep_history;
    config->RefreshIntervalMs = refresh_interval_ms;
    config->DefaultValueDivider = default_value_divider;
    config->DefaultValueMultiplier = default_value_multiplier;
    config->DefaultValueAdded = default_value_added;
    config->DefaultValuePrecision = default_value_precision;
  }

  snprintf(config->UnitBeforeValue, sizeof(config->UnitBeforeValue), "%s",
           unit_before_value.c_str());
  snprintf(config->UnitAfterValue, sizeof(config->UnitAfterValue), "%s",
           unit_after_value.c_str());

  snprintf(config->DefaultUnitBeforeValue,
           sizeof(config->DefaultUnitBeforeValue), "%s",
           default_unit_before_value.c_str());
  snprintf(config->DefaultUnitAfterValue, sizeof(config->DefaultUnitAfterValue),
           "%s", default_unit_after_value.c_str());

  string str_value;

  if (get_string(user_root, field_map.at(FIELD_METER_CHART_TYPE).c_str(),
                 &str_value)) {
    config->ChartType = string_to_chart_type(str_value);
    result = true;
  }

  bool bool_value = false;

  if (get_bool(user_root,
               field_map.at(FIELD_INCLUDE_VALUE_ADDED_IN_HISTORY).c_str(),
               &bool_value)) {
    config->IncludeValueAddedInHistory = bool_value ? 1 : 0;
    result = true;
  }

  if (get_bool(user_root, field_map.at(FIELD_FILL_MISSING_DATA).c_str(),
               &bool_value)) {
    config->FillMissingData = bool_value ? 1 : 0;
    result = true;
  }

  if (get_string(user_root, field_map.at(FIELD_COUNTER_TYPE).c_str(),
                 &str_value)) {
    config->CounterType = string_to_counter_type(str_value);
    result = true;
  }

  return result;
}
