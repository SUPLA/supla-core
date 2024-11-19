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

#include "json_helper.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_LEN 100

using std::map;
using std::string;
using std::stringstream;

supla_json_helper::supla_json_helper(void) {}

supla_json_helper::~supla_json_helper(void) {}

bool supla_json_helper::equal_ci(const char *str1, const char *str2) {
  if (!str1 || !str2) {
    return false;
  }

  size_t str_len = strnlen(str1, MAX_STR_LEN);

  if (str_len != strnlen(str2, MAX_STR_LEN)) {
    return false;
  }

  for (size_t a = 0; a < str_len; a++) {
    if (tolower(str1[a]) != tolower(str2[a])) {
      return false;
    }
  }

  return true;
}

bool supla_json_helper::equal_ci(cJSON *item, const char *str) {
  return item && equal_ci(cJSON_GetStringValue(item), str);
}

bool supla_json_helper::get_double(cJSON *parent, const char *key,
                                   double *value) {
  if (!parent || !key || !value) {
    return false;
  }

  cJSON *json_value = cJSON_GetObjectItem(parent, key);
  if (json_value && cJSON_IsNumber(json_value)) {
    *value = cJSON_GetNumberValue(json_value);
    return true;
  }

  *value = 0;
  return false;
}

bool supla_json_helper::get_bool(cJSON *parent, const char *key, bool *value) {
  if (!parent || !key || !value) {
    return false;
  }

  cJSON *json_value = cJSON_GetObjectItem(parent, key);
  if (json_value && cJSON_IsBool(json_value)) {
    *value = cJSON_IsTrue(json_value);
    return true;
  }

  *value = false;
  return false;
}

bool supla_json_helper::get_string(cJSON *parent, const char *key,
                                   std::string *value) {
  if (!parent || !key || !value) {
    return false;
  }

  cJSON *json_value = cJSON_GetObjectItem(parent, key);
  if (json_value && cJSON_IsString(json_value)) {
    char *str = cJSON_GetStringValue(json_value);
    *value = str ? str : "";
    return true;
  }

  return false;
}

cJSON *supla_json_helper::set_item_value(cJSON *parent, const std::string &name,
                                         int type, bool force, cJSON *obj,
                                         const char *string_value,
                                         double number_value) {
  if (!parent || name.empty()) {
    return nullptr;
  }

  cJSON *item_json = cJSON_GetObjectItem(parent, name.c_str());

  if (!item_json && !force) {
    return nullptr;
  }

  if (item_json && !force &&
      !(item_json->type == type ||
        (item_json->type == cJSON_True && type == cJSON_False) ||
        (item_json->type == cJSON_False && type == cJSON_True))) {
    return nullptr;
  }

  if (item_json && (item_json->type != type)) {
    cJSON_Delete(cJSON_DetachItemViaPointer(parent, item_json));
    item_json = nullptr;
  }

  if (item_json) {
    if (type == cJSON_Number) {
      cJSON_SetNumberValue(item_json, number_value);
    } else if (type == cJSON_String) {
      cJSON_SetValuestring(item_json, string_value);
    } else if (type == cJSON_Object) {
      if (cJSON_ReplaceItemInObject(parent, name.c_str(), obj)) {
        return obj;
      } else {
        return nullptr;
      }
    } else {
      return nullptr;
    }
  } else {
    switch (type) {
      case cJSON_False:
        item_json = cJSON_AddFalseToObject(parent, name.c_str());
        break;
      case cJSON_True:
        item_json = cJSON_AddTrueToObject(parent, name.c_str());
        break;
      case cJSON_Number:
        item_json = cJSON_AddNumberToObject(parent, name.c_str(), number_value);
        break;
      case cJSON_String:
        item_json = cJSON_AddStringToObject(parent, name.c_str(), string_value);
        break;
      case cJSON_Object:
        if (cJSON_AddItemToObject(parent, name.c_str(), obj)) {
          return obj;
        } else {
          return nullptr;
        }
        break;
      case cJSON_NULL:
        item_json = cJSON_AddNullToObject(parent, name.c_str());
        break;
      default:
        return nullptr;
    }
  }

  return item_json;
}

bool supla_json_helper::merge(cJSON *src_parent, cJSON *dst_parent,
                              const map<unsigned _supla_int16_t, string> &m,
                              bool delete_nonexistent) {
  bool dst_changed = false;

  if (!src_parent || !dst_parent) {
    return false;
  }

  for (auto it = m.cbegin(); it != m.cend(); ++it) {
    cJSON *src_item = cJSON_GetObjectItem(src_parent, it->second.c_str());
    cJSON *dst_item = cJSON_GetObjectItem(dst_parent, it->second.c_str());
    if (src_item == nullptr) {
      if (dst_item && delete_nonexistent) {
        cJSON_DetachItemViaPointer(dst_parent, dst_item);
        cJSON_Delete(dst_item);
        dst_changed = true;
      }
    } else if (!dst_item || !cJSON_Compare(src_item, dst_item, cJSON_True)) {
      cJSON *dup = cJSON_Duplicate(src_item, cJSON_True);
      if (dup) {
        if (dst_item) {
          cJSON_ReplaceItemViaPointer(dst_parent, dst_item, dup);
        } else {
          cJSON_AddItemToObject(dst_parent, it->second.c_str(), dup);
        }
        dst_changed = true;
      }
    }
  }

  return dst_changed;
}
