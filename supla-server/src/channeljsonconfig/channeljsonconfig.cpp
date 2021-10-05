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

#include "channeljsonconfig.h"

channel_json_config::channel_json_config(void) {
  this->properties_root = NULL;
  this->user_root = NULL;
  this->root = NULL;
}

channel_json_config::channel_json_config(channel_json_config *root) {
  this->properties_root = NULL;
  this->user_root = NULL;
  this->root = root;
}

channel_json_config::~channel_json_config(void) {
  if (properties_root) {
    cJSON_Delete(properties_root);
    properties_root = NULL;
  }

  if (user_root) {
    cJSON_Delete(user_root);
    user_root = NULL;
  }
}

// User

cJSON *channel_json_config::get_user_root(void) {
  if (root) {
    return root->get_user_root();
  }

  if (user_root == NULL) {
    user_root = cJSON_CreateObject();
  }

  return user_root;
}

void channel_json_config::set_user_config(const char *config) {
  if (root) {
    root->set_user_config(config);
    return;
  }

  if (user_root != NULL) {
    cJSON_Delete(user_root);
    user_root = NULL;
  }

  if (config) {
    user_root = cJSON_Parse(config);
  } else {
    cJSON_Delete(user_root);
    user_root = NULL;
  }
}

char *channel_json_config::get_user_config(void) {
  cJSON *json = get_user_root();
  if (json) {
    return cJSON_PrintUnformatted(json);
  }
  return NULL;
}

// Properties

cJSON *channel_json_config::get_properties_root(void) {
  if (root) {
    return root->get_properties_root();
  }

  if (properties_root == NULL) {
    properties_root = cJSON_CreateObject();
  }

  return properties_root;
}

void channel_json_config::set_properties(const char *properties) {
  if (root) {
    root->set_properties(properties);
    return;
  }

  if (properties_root != NULL) {
    cJSON_Delete(properties_root);
    properties_root = NULL;
  }

  if (properties) {
    properties_root = cJSON_Parse(properties);
  } else {
    cJSON_Delete(properties_root);
    properties_root = NULL;
  }
}

char *channel_json_config::get_properties(void) {
  cJSON *json = get_properties_root();
  if (json) {
    return cJSON_PrintUnformatted(json);
  }
  return NULL;
}
