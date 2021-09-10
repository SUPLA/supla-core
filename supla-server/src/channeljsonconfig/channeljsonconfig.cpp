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
  this->json_root = NULL;
  this->root = NULL;
}

channel_json_config::channel_json_config(channel_json_config *root) {
  this->json_root = NULL;
  this->root = root;
}

channel_json_config::~channel_json_config(void) {
  if (json_root) {
    cJSON_Delete(json_root);
    json_root = NULL;
  }
}

cJSON *channel_json_config::get_json_root(void) {
  if (root) {
    return root->get_json_root();
  }

  if (json_root == NULL) {
    json_root = cJSON_CreateObject();
  }

  return json_root;
}

void channel_json_config::set_config(const char *config) {
  if (root) {
    root->set_config(config);
    return;
  }

  if (json_root != NULL) {
    cJSON_Delete(json_root);
    json_root = NULL;
  }

  if (config) {
    json_root = cJSON_Parse(config);
  }
}

char *channel_json_config::get_config(void) {
  cJSON *json = get_json_root();
  if (json) {
    return cJSON_PrintUnformatted(json);
  }
  return NULL;
}
