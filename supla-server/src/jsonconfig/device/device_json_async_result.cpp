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

#include "device_json_async_result.h"

#include <string.h>

#include "log.h"

using std::map;
using std::string;

device_json_async_result::device_json_async_result(void)
    : supla_json_config() {}

device_json_async_result::device_json_async_result(supla_json_config *root)
    : supla_json_config(root) {}

device_json_async_result::~device_json_async_result(void) {}

__time_t device_json_async_result::get_timestamp(void) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  return now.tv_sec;
}

cJSON *device_json_async_result::create_timestamp_object(void) {
  cJSON *obj = cJSON_CreateObject();
  if (obj) {
    cJSON_AddNumberToObject(obj, "timestamp", get_timestamp());
  }

  return obj;
}

cJSON *device_json_async_result::create_status_object(string status) {
  cJSON *obj = create_timestamp_object();
  if (obj) {
    cJSON_AddStringToObject(obj, "status" status.c_str());
  }

  return obj;
}

void device_json_async_result::apply_on_properties_root(cJSON *ota) {
  if (ota) {
    cJSON *properties_root = get_properties_root();
    if (properties_root) {
      set_item_value(properties_root, get_status_root_key(), cJSON_Object, true,
                     ota, nullptr, 0);
    }
  }
}

void device_json_async_result::merge(supla_json_config *_dst) {
  if (dynamic_cast<device_json_async_result *>(_dst)) {
    supla_json_config::merge(
        get_properties_root(),
        dynamic_cast<device_json_async_result *>(_dst)->get_properties_root(),
        get_field_map(), false);
  }
}
