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

#define FIELD_METER_CHART_TYPE 1
#define FIELD_INCLUDE_VALUE_ADDED_IN_HISTORY 2
#define FIELD_FILL_MISSING_DATA 3
#define FIELD_ALLOW_COUNTER_RESET 4

const map<unsigned _supla_int16_t, string>
    general_purpose_meter_config::field_map = {
        {FIELD_METER_CHART_TYPE, "chartType"},
        {FIELD_INCLUDE_VALUE_ADDED_IN_HISTORY, "includeValueAddedInHistory"},
        {FIELD_FILL_MISSING_DATA, "fillMissingData"},
        {FIELD_ALLOW_COUNTER_RESET, "allowCounterReset"}};

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
  if (chart_type == SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_BAR) {
    return "BAR";
  }

  return "LINEAR";
}

unsigned char general_purpose_meter_config::string_to_chart_type(
    const string &chart_type) {
  if (chart_type == "BAR") {
    return SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_BAR;
  }

  return SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_LINEAR;
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
      config->KeepHistory, config->DefaultValueDivider,
      config->DefaultValueMultiplier, config->DefaultValueAdded,
      config->DefaultValuePrecision, default_unit_before_value,
      default_unit_after_value);

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

  set_item_value(user_root, field_map.at(FIELD_ALLOW_COUNTER_RESET).c_str(),
                 config->AllowCounterReset ? cJSON_True : cJSON_False, true,
                 nullptr, nullptr, 0);
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

  result = general_purpose_base_config::get_config(
      &config->ValueDivider, &config->ValueMultiplier, &config->ValueAdded,
      &config->ValuePrecision, &unit_before_value, &unit_after_value,
      &config->KeepHistory, &config->DefaultValueDivider,
      &config->DefaultValueMultiplier, &config->DefaultValueAdded,
      &config->DefaultValuePrecision, &default_unit_before_value,
      &default_unit_after_value);

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

  if (get_bool(user_root, field_map.at(FIELD_ALLOW_COUNTER_RESET).c_str(),
               &bool_value)) {
    config->AllowCounterReset = bool_value ? 1 : 0;
    result = true;
  }

  return result;
}
