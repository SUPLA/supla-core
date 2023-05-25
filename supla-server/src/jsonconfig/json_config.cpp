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

int supla_json_config::get_map_size(void) { return 0; }

int supla_json_config::get_map_key(int index) { return 0; }

const char *supla_json_config::get_map_str(int index) { return nullptr; }

bool supla_json_config::key_exists(int key) {
  int size = get_map_size();

  for (int a = 0; a < size; a++) {
    if (get_map_key(a) == key) {
      return true;
    }
  }

  return false;
}

const char *supla_json_config::string_with_key(int key) {
  int size = get_map_size();

  for (int a = 0; a < size; a++) {
    if (get_map_key(a) == key) {
      return get_map_str(a);
    }
  }

  return nullptr;
}

int supla_json_config::key_with_string(const char *str) {
  int size = get_map_size();

  for (int a = 0; a < size; a++) {
    if (equal(get_map_str(a), str)) {
      return get_map_key(a);
    }
  }

  return 0;
}

int supla_json_config::json_to_key(cJSON *item) {
  if (item) {
    return key_with_string(cJSON_GetStringValue(item));
  }

  return 0;
}

bool supla_json_config::equal(const char *str1, const char *str2) {
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

bool supla_json_config::equal(cJSON *item, const char *str) {
  return item && equal(cJSON_GetStringValue(item), str);
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
