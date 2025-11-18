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

#include "power_switch_config.h"

#include "json/cJSON.h"

using std::map;
using std::string;

#define FIELD_OVERCURRENT_THRESHOLD 1
#define FIELD_OVERCURRENT_MAX_ALLOWED 2
#define FIELD_DEFAULT_RELATED_MASTER_CHANNEL_NO 3
#define FIELD_RELAY_TIME 4

const map<unsigned _supla_int16_t, string> power_switch_config::field_map = {
    {FIELD_OVERCURRENT_THRESHOLD, "overcurrentThreshold"},
    {FIELD_OVERCURRENT_MAX_ALLOWED, "overcurrentMaxAllowed"},
    {FIELD_DEFAULT_RELATED_MASTER_CHANNEL_NO, "defaultRelatedMeterChannelNo"},
    {FIELD_RELAY_TIME, "relayTime"}};

const char power_switch_config::related_meter_channel_id_field[] =
    "relatedMeterChannelId";

power_switch_config::power_switch_config(void) : supla_json_config() {}

power_switch_config::power_switch_config(supla_json_config *root)
    : supla_json_config(root) {}

power_switch_config::~power_switch_config(void) {}

void power_switch_config::set_config(
    TChannelConfig_PowerSwitch *config,
    supla_abstract_common_channel_properties *props) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  cJSON *properties_root = get_properties_root();
  if (!properties_root) {
    return;
  }

  set_item_value(user_root, field_map.at(FIELD_OVERCURRENT_THRESHOLD).c_str(),
                 cJSON_Number, true, nullptr, nullptr,
                 config->OvercurrentThreshold);

  set_item_value(
      properties_root, field_map.at(FIELD_OVERCURRENT_MAX_ALLOWED).c_str(),
      cJSON_Number, true, nullptr, nullptr, config->OvercurrentMaxAllowed);

  set_item_value(properties_root,
                 field_map.at(FIELD_DEFAULT_RELATED_MASTER_CHANNEL_NO).c_str(),
                 config->DefaultRelatedMeterIsSet ? cJSON_Number : cJSON_NULL,
                 true, nullptr, nullptr, config->DefaultRelatedMeterChannelNo);

  double dbl_value = 0;
  int channel_id = 0;

  if (props && config->DefaultRelatedMeterIsSet &&
      !get_double(user_root, related_meter_channel_id_field, &dbl_value) &&
      config->DefaultRelatedMeterChannelNo != props->get_channel_number() &&
      (channel_id =
           props->get_channel_id(config->DefaultRelatedMeterChannelNo))) {
    set_item_value(user_root, related_meter_channel_id_field, cJSON_Number,
                   true, nullptr, nullptr, channel_id);
  }
}

void power_switch_config::set_config(
    TChannelConfig_StaircaseTimer *config,
    supla_abstract_common_channel_properties *props) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  set_item_value(user_root, field_map.at(FIELD_RELAY_TIME).c_str(),
                 cJSON_Number, true, nullptr, nullptr, config->TimeMS / 100);
}

bool power_switch_config::get_config(TChannelConfig_PowerSwitch *config) {
  if (!config) {
    return false;
  }

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

  if (get_double(user_root, field_map.at(FIELD_OVERCURRENT_THRESHOLD).c_str(),
                 &dbl_value)) {
    config->OvercurrentThreshold = dbl_value;
    result = true;
  } else {
    config->OvercurrentThreshold = 0;
  }

  if (get_double(properties_root,
                 field_map.at(FIELD_OVERCURRENT_MAX_ALLOWED).c_str(),
                 &dbl_value)) {
    config->OvercurrentMaxAllowed = dbl_value;
    result = true;
  } else {
    config->OvercurrentMaxAllowed = 0;
  }

  if (get_double(properties_root,
                 field_map.at(FIELD_DEFAULT_RELATED_MASTER_CHANNEL_NO).c_str(),
                 &dbl_value)) {
    config->DefaultRelatedMeterChannelNo = dbl_value;
    config->DefaultRelatedMeterIsSet = 1;
    result = true;
  } else {
    config->DefaultRelatedMeterChannelNo = 0;
    config->DefaultRelatedMeterIsSet = 0;
  }

  return result;
}

bool power_switch_config::get_config(TChannelConfig_StaircaseTimer *config) {
  if (!config) {
    return false;
  }

  bool result = false;

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  double dbl_value = 0;

  if (get_double(user_root, field_map.at(FIELD_RELAY_TIME).c_str(),
                 &dbl_value)) {
    config->TimeMS = dbl_value * 100;
    result = true;
  } else {
    config->TimeMS = 0;
  }

  return result;
}

void power_switch_config::merge(supla_json_config *_dst) {
  power_switch_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           false);

  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           field_map, false);

  if (!cJSON_GetObjectItem(dst.get_user_root(),
                           related_meter_channel_id_field)) {
    std::map<unsigned _supla_int16_t, std::string> field_map = {
        {1, related_meter_channel_id_field}};
    supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                             false);
  }
}

int power_switch_config::get_related_meter_channel_id(void) {
  cJSON *user_root = get_user_root();
  if (user_root) {
    double dbl_value = 0;
    if (get_double(user_root, related_meter_channel_id_field, &dbl_value)) {
      return dbl_value;
    }
  }

  return 0;
}

int power_switch_config::get_relay_time_ms(void) {
  cJSON *user_root = get_user_root();
  if (user_root) {
    double dbl_value = 0;
    if (get_double(user_root, field_map.at(FIELD_RELAY_TIME).c_str(),
                   &dbl_value)) {
      return dbl_value * 100;
    }
  }

  return 0;
}
