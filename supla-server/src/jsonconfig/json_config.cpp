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

#include "json_config.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define MAX_STR_LEN 100

using std::map;
using std::string;
using std::stringstream;

supla_json_config::supla_json_config(void) {
  this->user_root = nullptr;
  this->root = nullptr;
}

supla_json_config::supla_json_config(const supla_json_config &json_config) {
  this->user_root = nullptr;
  this->root = nullptr;

  *this = json_config;
}

supla_json_config::supla_json_config(supla_json_config *root) {
  this->user_root = nullptr;
  this->root = root;
}

supla_json_config::supla_json_config(supla_json_config *root,
                                     bool copy_and_detach) {
  this->user_root = nullptr;
  this->root = root;

  if (root && copy_and_detach) {
    this->user_root = cJSON_Duplicate(root->get_user_root(), cJSON_True);
    this->root = nullptr;
  }
}

supla_json_config::~supla_json_config(void) {
  if (this->user_root) {
    cJSON_Delete(this->user_root);
    this->user_root = nullptr;
  }
}

supla_json_config *supla_json_config::get_root(void) { return root; }

// User

cJSON *supla_json_config::get_user_root(void) {
  if (root) {
    return root->get_user_root();
  }

  if (user_root == nullptr) {
    user_root = cJSON_CreateObject();
  }

  return user_root;
}

bool supla_json_config::is_root_exists(void) { return root != nullptr; }

void supla_json_config::set_user_config(const char *config) {
  if (root) {
    root->set_user_config(config);
    return;
  }

  if (user_root != nullptr) {
    cJSON_Delete(user_root);
    user_root = nullptr;
  }

  if (config) {
    user_root = cJSON_Parse(config);
  }
}

char *supla_json_config::get_user_config(void) {
  cJSON *json = get_user_root();
  if (json) {
    return cJSON_PrintUnformatted(json);
  }
  return nullptr;
}

bool supla_json_config::equal_ci(const char *str1, const char *str2) {
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

bool supla_json_config::equal_ci(cJSON *item, const char *str) {
  return item && equal_ci(cJSON_GetStringValue(item), str);
}

int supla_json_config::get_int(const char *key) {
  cJSON *root = get_user_root();
  if (!root) {
    return 0;
  }

  cJSON *value = cJSON_GetObjectItem(root, key);
  if (value && cJSON_IsNumber(value)) {
    return value->valueint;
  }

  return 0;
}

double supla_json_config::get_double(const char *key) {
  cJSON *root = get_user_root();
  if (!root) {
    return false;
  }

  cJSON *value = cJSON_GetObjectItem(root, key);
  if (value && cJSON_IsNumber(value)) {
    return value->valuedouble;
  }

  return 0;
}

bool supla_json_config::get_bool(const char *key) {
  cJSON *root = get_user_root();
  if (!root) {
    return false;
  }

  cJSON *value = cJSON_GetObjectItem(root, key);
  return value && cJSON_IsBool(value) && cJSON_IsTrue(value);
}

cJSON *supla_json_config::set_item_value(cJSON *parent, const std::string &name,
                                         int type, bool force,
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
      case cJSON_NULL:
        item_json = cJSON_AddNullToObject(parent, name.c_str());
        break;
      default:
        return nullptr;
    }
  }

  return item_json;
}

bool supla_json_config::merge(cJSON *src_parent, cJSON *dst_parent,
                              const map<int, string> &m) {
  bool dst_changed = false;

  for (auto it = m.cbegin(); it != m.cend(); ++it) {
    cJSON *src_item = cJSON_GetObjectItem(src_parent, it->second.c_str());
    cJSON *dst_item = cJSON_GetObjectItem(dst_parent, it->second.c_str());
    if (src_item == nullptr) {
      if (dst_item) {
        cJSON_DetachItemViaPointer(dst_parent, dst_item);
        cJSON_Delete(dst_item);
      }
    } else if (!dst_item || !cJSON_Compare(src_item, dst_item, cJSON_True)) {
      set_item_value(dst_parent, it->second, src_item->type, true,
                     cJSON_GetStringValue(src_item),
                     cJSON_GetNumberValue(src_item));
      dst_changed = true;
    }
  }

  return dst_changed;
}

supla_json_config &supla_json_config::operator=(
    const supla_json_config &json_config) {
  if (this->user_root) {
    cJSON_Delete(this->user_root);
    this->user_root = nullptr;
  }

  this->root = json_config.root;

  if (json_config.user_root) {
    this->user_root = cJSON_Duplicate(json_config.user_root, cJSON_True);
  }

  return *this;
}
