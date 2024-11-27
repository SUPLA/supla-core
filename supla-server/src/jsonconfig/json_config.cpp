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

#include <functional>
#include <string>

#include "log.h"

#define MAX_STR_LEN 100

using std::function;
using std::map;
using std::string;
using std::stringstream;

supla_json_config::supla_json_config(void) {
  this->user_root = nullptr;
  this->properties_root = nullptr;
  this->root = nullptr;
}

supla_json_config::supla_json_config(const supla_json_config &json_config)
    : supla_json_helper() {
  this->user_root = nullptr;
  this->properties_root = nullptr;
  this->root = nullptr;

  *this = json_config;
}

supla_json_config::supla_json_config(supla_json_config *root)
    : supla_json_helper() {
  this->user_root = nullptr;
  this->properties_root = nullptr;
  this->root = root;
}

supla_json_config::supla_json_config(supla_json_config *root,
                                     bool copy_and_detach)
    : supla_json_helper() {
  this->user_root = nullptr;
  this->properties_root = nullptr;
  this->root = root;

  if (root && copy_and_detach) {
    this->user_root = cJSON_Duplicate(root->get_user_root(), cJSON_True);
    this->properties_root =
        cJSON_Duplicate(root->get_properties_root(), cJSON_True);
    this->root = nullptr;
  }
}

void supla_json_config::json_clear(void) {
  if (this->user_root) {
    cJSON_Delete(this->user_root);
    this->user_root = nullptr;
  }

  if (this->properties_root) {
    cJSON_Delete(this->properties_root);
    this->properties_root = nullptr;
  }
}

supla_json_config::~supla_json_config(void) { json_clear(); }

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

cJSON *supla_json_config::get_properties_root(void) {
  supla_json_config *root = dynamic_cast<supla_json_config *>(get_root());

  if (root) {
    return root->get_properties_root();
  }

  if (properties_root == nullptr) {
    properties_root = cJSON_CreateObject();
  }

  return properties_root;
}

void supla_json_config::set_properties(const char *properties) {
  supla_json_config *root = dynamic_cast<supla_json_config *>(get_root());
  if (root) {
    root->set_properties(properties);
    return;
  }

  if (properties_root != nullptr) {
    cJSON_Delete(properties_root);
    properties_root = nullptr;
  }

  if (properties) {
    properties_root = cJSON_Parse(properties);
  }
}

char *supla_json_config::get_properties(void) {
  cJSON *json = get_properties_root();
  if (json) {
    return cJSON_PrintUnformatted(json);
  }
  return nullptr;
}

cJSON *supla_json_config::get_user_root_with_key(const char *key, bool force) {
  cJSON *root = get_user_root();
  if (root) {
    cJSON *key_root = cJSON_GetObjectItem(root, key);
    if (!key_root) {
      key_root = cJSON_AddObjectToObject(root, key);
    }
    return key_root;
  }
  return nullptr;
}

double supla_json_config::get_double(const char *key) {
  double result = 0;
  supla_json_helper::get_double(get_user_root(), key, &result);

  return result;
}

bool supla_json_config::get_double(cJSON *parent, const char *key,
                                   double *value) {
  return supla_json_helper::get_double(parent, key, value);
}

bool supla_json_config::get_bool(const char *key) {
  bool result = false;
  supla_json_helper::get_bool(get_user_root(), key, &result);
  return result;
}

bool supla_json_config::get_bool(cJSON *parent, const char *key, bool *value) {
  return supla_json_helper::get_bool(parent, key, value);
}

supla_json_config &supla_json_config::operator=(
    const supla_json_config &json_config) {
  json_clear();

  this->root = json_config.root;

  if (json_config.user_root) {
    this->user_root = cJSON_Duplicate(json_config.user_root, cJSON_True);
  }

  if (json_config.properties_root) {
    this->properties_root =
        cJSON_Duplicate(json_config.properties_root, cJSON_True);
  }

  return *this;
}

bool supla_json_config::merge(cJSON *src_parent, cJSON *dst_parent,
                              const map<unsigned _supla_int16_t, string> &m,
                              bool delete_nonexistent) {
  return supla_json_helper::merge(src_parent, dst_parent, m,
                                  delete_nonexistent);
}

void supla_json_config::merge(supla_json_config *dst) {
  throw std::bad_function_call();
}
