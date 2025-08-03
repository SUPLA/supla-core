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

#include "device_json_ota_updates.h"

#include <string.h>

#include "log.h"

#define FIELD_OTA_UPDATE 1

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string> device_json_ota_updates::field_map =
    {{FIELD_OTA_UPDATE, "otaUpdate"}};

device_json_ota_updates::device_json_ota_updates(void) : supla_json_config() {}

device_json_ota_updates::device_json_ota_updates(supla_json_config *root)
    : supla_json_config(root) {}

device_json_ota_updates::~device_json_ota_updates(void) {}

__time_t device_json_ota_updates::get_timestamp(void) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  return now.tv_sec;
}

cJSON *device_json_ota_updates::create_status_object(string status) {
  cJSON *ota = cJSON_CreateObject();
  if (ota) {
    cJSON_AddNumberToObject(ota, "timestamp", get_timestamp());
    cJSON_AddStringToObject(ota, "status", status.c_str());
  }

  return ota;
}

void device_json_ota_updates::apply_on_properties_root(cJSON *ota) {
  if (ota) {
    cJSON *properties_root = get_properties_root();
    if (properties_root) {
      set_item_value(properties_root, field_map.at(FIELD_OTA_UPDATE),
                     cJSON_Object, true, ota, nullptr, 0);
    }
  }
}

void device_json_ota_updates::set_checking(void) {
  apply_on_properties_root(create_status_object("checking"));
}

void device_json_ota_updates::set_not_available(void) {
  apply_on_properties_root(create_status_object("notAvailable"));
}

void device_json_ota_updates::set_available(string soft_ver,
                                            string changelog_url) {
  cJSON *ota = create_status_object("available");
  if (ota) {
    cJSON_AddStringToObject(ota, "version", soft_ver.c_str());
    cJSON_AddStringToObject(ota, "url", changelog_url.c_str());
    apply_on_properties_root(ota);
  }
}

void device_json_ota_updates::set_error(void) {
  apply_on_properties_root(create_status_object("error"));
}

void device_json_ota_updates::merge(supla_json_config *_dst) {
  device_json_ota_updates dst(_dst);

  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           field_map, false);
}
