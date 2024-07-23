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

#include "ocr_config.h"

#include <vector>

#include "svrcfg.h"
#include "tools.h"

using std::map;
using std::string;
using std::vector;

#define FIELD_AUTHKEY 1
#define FIELD_PHOTO_INTERVAL_SEC 2
#define FIELD_LIGHTING_MODE 3
#define FIELD_LIGHTING_LEVEL 4
#define FIELD_AVAILABLE_LGHTING_MODES 5

const map<unsigned _supla_int16_t, string> ocr_config::field_map = {
    {FIELD_AUTHKEY, "authKey"},
    {FIELD_PHOTO_INTERVAL_SEC, "photoIntervalSec"},
    {FIELD_LIGHTING_MODE, "lightingMode"},
    {FIELD_LIGHTING_LEVEL, "lightingLevel"},
    {FIELD_AVAILABLE_LGHTING_MODES, "availableLightingModes"}};

ocr_config::ocr_config(supla_json_config *root) : supla_json_config(root) {}

ocr_config::ocr_config(void) : supla_json_config() {}

cJSON *ocr_config::get_ocr_root(cJSON *root, bool force) {
  if (!root) {
    return nullptr;
  }

  const char key[] = "ocr";

  cJSON *ocr = cJSON_GetObjectItem(root, key);
  if (!ocr && force) {
    ocr = cJSON_AddObjectToObject(root, key);
  }

  return ocr;
}

cJSON *ocr_config::_get_user_root(void) {
  return get_ocr_root(supla_json_config::get_user_root(), true);
}

cJSON *ocr_config::_get_properties_root(bool force) {
  return get_ocr_root(supla_json_config::get_properties_root(), force);
}

void ocr_config::merge(supla_json_config *_dst) {
  map<unsigned _supla_int16_t, string> field_map = ocr_config::field_map;

  ocr_config dst(_dst);
  cJSON *dst_properties_root = dst._get_properties_root(false);

  if (dst_properties_root) {
    std::string str_value;
    if (get_string(dst_properties_root, field_map.at(FIELD_AUTHKEY).c_str(),
                   &str_value) &&
        str_value.size()) {
      field_map.erase(FIELD_AUTHKEY);
    }
  }

  supla_json_config::merge(_get_user_root(), dst._get_user_root(), field_map,
                           true);
  supla_json_config::merge(_get_properties_root(), dst._get_properties_root(),
                           field_map, true);
}

string ocr_config::lighting_mode_to_string(unsigned char mode) {
  switch (mode) {
    case OCR_LIGHTING_MODE_OFF:
      return "OFF";
    case OCR_LIGHTING_MODE_ALWAYS_ON:
      return "ALWAYS_ON";
    case OCR_LIGHTING_MODE_AUTO:
      return "AUTO";
  }

  return "";
}

unsigned char ocr_config::string_lighting_mode(std::string mode) {
  if (mode == "OFF") {
    return OCR_LIGHTING_MODE_OFF;
  } else if (mode == "ALWAYS_ON") {
    return OCR_LIGHTING_MODE_ALWAYS_ON;
  } else if (mode == "AUTO") {
    return OCR_LIGHTING_MODE_AUTO;
  }

  return 0;
}

void ocr_config::set_config(TChannelConfig_OCR *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = _get_user_root();
  if (!user_root) {
    return;
  }

  unsigned int min_interval_sec = scfg_int(CFG_OCR_MIN_INTERVAL_SEC);

  set_item_value(user_root, field_map.at(FIELD_PHOTO_INTERVAL_SEC).c_str(),
                 cJSON_Number, true, nullptr, nullptr,
                 config->PhotoIntervalSec < min_interval_sec &&
                         config->PhotoIntervalSec > 0
                     ? min_interval_sec
                     : config->PhotoIntervalSec);

  set_item_value(user_root, field_map.at(FIELD_LIGHTING_MODE).c_str(),
                 cJSON_String, true, nullptr,
                 lighting_mode_to_string(config->LightingMode).c_str(), 0);

  if (config->LightingLevel >= 1 && config->LightingLevel <= 100) {
    set_item_value(user_root, field_map.at(FIELD_LIGHTING_LEVEL).c_str(),
                   cJSON_Number, true, nullptr, nullptr, config->LightingLevel);
  }

  cJSON *properties_root = _get_properties_root();
  if (!properties_root) {
    return;
  }

  cJSON *modes = cJSON_GetObjectItem(
      properties_root, field_map.at(FIELD_AVAILABLE_LGHTING_MODES).c_str());

  if (modes) {
    cJSON_DetachItemViaPointer(properties_root, modes);
    cJSON_Delete(modes);
  }

  modes = cJSON_AddArrayToObject(
      properties_root, field_map.at(FIELD_AVAILABLE_LGHTING_MODES).c_str());

  std::vector<_supla_int64_t> modes_raw = {OCR_LIGHTING_MODE_OFF,
                                           OCR_LIGHTING_MODE_ALWAYS_ON,
                                           OCR_LIGHTING_MODE_AUTO};

  for (auto it = modes_raw.cbegin(); it != modes_raw.cend(); ++it) {
    if (config->AvailableLightingModes & (*it)) {
      cJSON *mode_json =
          cJSON_CreateString(lighting_mode_to_string(*it).c_str());
      if (mode_json) {
        cJSON_AddItemToArray(modes, mode_json);
      }
    }
  }

  char auth_key[SUPLA_OCR_AUTHKEY_SIZE] = {};
  st_random_alpha_string(auth_key, SUPLA_OCR_AUTHKEY_SIZE);

  set_item_value(properties_root, field_map.at(FIELD_AUTHKEY).c_str(),
                 cJSON_String, true, nullptr, auth_key, 0);
}

bool ocr_config::get_config(TChannelConfig_OCR *config) {
  bool result = false;

  if (!config) {
    return result;
  }

  snprintf(config->Host, SUPLA_URL_HOST_MAXSIZE, "%s",
           scfg_string(CFG_OCR_HOST));

  cJSON *user_root = _get_user_root();
  if (!user_root) {
    return result;
  }

  std::string str_value;

  double dbl_value = 0;

  if (get_double(user_root, field_map.at(FIELD_PHOTO_INTERVAL_SEC).c_str(),
                 &dbl_value)) {
    config->PhotoIntervalSec = dbl_value;
    result = true;
  }

  if (get_string(user_root, field_map.at(FIELD_LIGHTING_MODE).c_str(),
                 &str_value)) {
    config->LightingMode = string_lighting_mode(str_value);
    result = true;
  }

  if (get_double(user_root, field_map.at(FIELD_LIGHTING_LEVEL).c_str(),
                 &dbl_value) &&
      dbl_value >= 0) {
    config->LightingLevel = dbl_value;
    result = true;
  }

  cJSON *properties_root = _get_properties_root();
  if (!properties_root) {
    return false;
  }

  cJSON *modes = cJSON_GetObjectItem(
      properties_root, field_map.at(FIELD_AVAILABLE_LGHTING_MODES).c_str());

  if (modes && cJSON_IsArray(modes)) {
    for (int a = 0; a < cJSON_GetArraySize(modes); a++) {
      cJSON *item = cJSON_GetArrayItem(modes, a);
      if (item && cJSON_IsString(item)) {
        config->AvailableLightingModes |=
            string_lighting_mode(cJSON_GetStringValue(item));
        result = true;
      }
    }
  }

  if (get_string(properties_root, field_map.at(FIELD_AUTHKEY).c_str(),
                 &str_value)) {
    snprintf(config->AuthKey, SUPLA_OCR_AUTHKEY_SIZE, "%s", str_value.c_str());
    result = true;
  }

  return result;
}
