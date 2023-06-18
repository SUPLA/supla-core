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

#include "channel_json_config.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

channel_json_config::channel_json_config(void) : supla_json_config() {
  this->properties_root = nullptr;
}

channel_json_config::channel_json_config(const channel_json_config &json_config)
    : supla_json_config(json_config) {
  this->properties_root = nullptr;
  *this = json_config;
}

channel_json_config::channel_json_config(supla_json_config *root)
    : supla_json_config(root) {
  this->properties_root = nullptr;
}

channel_json_config::channel_json_config(channel_json_config *root,
                                         bool copy_and_detach)
    : supla_json_config(root, copy_and_detach) {
  this->properties_root = nullptr;

  if (root && copy_and_detach) {
    this->properties_root =
        cJSON_Duplicate(root->get_properties_root(), cJSON_True);
  }
}

channel_json_config::~channel_json_config(void) {
  if (this->properties_root) {
    cJSON_Delete(this->properties_root);
    this->properties_root = nullptr;
  }
}

cJSON *channel_json_config::get_properties_root(void) {
  channel_json_config *root = dynamic_cast<channel_json_config *>(get_root());

  if (root) {
    return root->get_properties_root();
  }

  if (properties_root == nullptr) {
    properties_root = cJSON_CreateObject();
  }

  return properties_root;
}

void channel_json_config::set_properties(const char *properties) {
  channel_json_config *root = dynamic_cast<channel_json_config *>(get_root());
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

char *channel_json_config::get_properties(void) {
  cJSON *json = get_properties_root();
  if (json) {
    return cJSON_PrintUnformatted(json);
  }
  return nullptr;
}

channel_json_config &channel_json_config::operator=(
    const channel_json_config &json_config) {
  supla_json_config::operator=(json_config);

  if (this->properties_root) {
    cJSON_Delete(this->properties_root);
    this->properties_root = nullptr;
  }

  if (json_config.properties_root) {
    this->properties_root =
        cJSON_Duplicate(json_config.properties_root, cJSON_True);
  }

  return *this;
}
