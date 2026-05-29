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

#include "general_purpose_text_config.h"

#include <map>
#include <string>

using std::map;
using std::string;

#define FIELD_KEEP_HISTORY 1
#define FIELD_REFRESH_INTERVAL_MS 2

const map<unsigned _supla_int16_t, string>
    general_purpose_text_config::field_map = {
        {FIELD_KEEP_HISTORY, "keepHistory"},
        {FIELD_REFRESH_INTERVAL_MS, "refreshIntervalMs"}};

general_purpose_text_config::general_purpose_text_config(
    supla_json_config *root)
    : supla_json_config(root) {}

general_purpose_text_config::general_purpose_text_config(void)
    : supla_json_config() {}

void general_purpose_text_config::merge(supla_json_config *_dst) {
  general_purpose_text_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           true);
  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           field_map, true);
}

void general_purpose_text_config::set_config(
    TChannelConfig_GeneralPurposeText *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  set_item_value(user_root,
                 field_map.at(FIELD_KEEP_HISTORY).c_str(),
                 cJSON_Number, true, nullptr, nullptr,
                 config->KeepHistory ? 1 : 0);

  set_item_value(user_root,
                 field_map.at(FIELD_REFRESH_INTERVAL_MS).c_str(),
                 cJSON_Number, true, nullptr, nullptr,
                 config->RefreshIntervalMs);
}

bool general_purpose_text_config::get_config(
    TChannelConfig_GeneralPurposeText *config) {
  bool result = false;

  if (!config) {
    return result;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return result;
  }

  double dbl = 0;

  if (get_double(user_root, field_map.at(FIELD_KEEP_HISTORY).c_str(), &dbl)) {
    config->KeepHistory = dbl >= 1 ? 1 : 0;
    result = true;
  }

  if (get_double(user_root,
                 field_map.at(FIELD_REFRESH_INTERVAL_MS).c_str(), &dbl)) {
    config->RefreshIntervalMs =
        static_cast<unsigned _supla_int16_t>(dbl);
    result = true;
  }

  return result;
}

bool general_purpose_text_config::keep_history(void) {
  TChannelConfig_GeneralPurposeText config = {};
  get_config(&config);
  return config.KeepHistory == 1;
}
