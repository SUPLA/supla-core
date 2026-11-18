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

#include <mqtt_device_message_provider.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "cJSON.h"

supla_mqtt_device_message_provider::supla_mqtt_device_message_provider(void)
    : supla_mqtt_message_provider() {
  row = NULL;
}

supla_mqtt_device_message_provider::~supla_mqtt_device_message_provider(void) {}

bool supla_mqtt_device_message_provider::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (row != NULL) {
    switch (index) {
      case 0:
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size,
                              row->device_enabled ? "true" : "false", false,
                              "devices/%i/enabled", row->device_id);
      case 1:
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, row->device_last_connected, false,
                              "devices/%i/last_connected", row->device_id);
      case 2:
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, row->device_last_ipv4, false,
                              "devices/%i/last_ipv4", row->device_id);
      case 3: {
        char mfr_name[20];
        get_mfr_name(row->device_mfr_id, mfr_name, sizeof(mfr_name));
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, mfr_name, false,
                              "devices/%i/manufacturer", row->device_id);
      }

      case 4:
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, row->device_name, false,
                              "devices/%i/name", row->device_id);
      case 5: {
        char proto_ver[15];
        snprintf(proto_ver, sizeof(proto_ver), "%i", row->device_proto_version);

        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, proto_ver, false,
                              "devices/%i/proto_ver", row->device_id);
      }
      case 6:
        return create_message(topic_prefix, row->user_suid, topic_name, message,
                              message_size, row->device_softver, false,
                              "devices/%i/soft_ver", row->device_id);
      case 7: {
        if (!topic_name) {
          return false;
        }

        const auto id = std::to_string(row->device_id);
        // Keep the original Discovery ID for existing entities.
        const auto object_id = "gateway_" + id + "_last_connected";
        const auto discovery_topic =
            std::string("homeassistant/sensor/") + row->user_suid + "/" +
            object_id + "/config";
        if (!message || !row->device_enabled) {
          *topic_name = strdup(discovery_topic.c_str());
          return *topic_name != nullptr;
        }

        char *state_topic = nullptr;
        if (!create_message(topic_prefix, row->user_suid, &state_topic, nullptr,
                            nullptr, nullptr, false,
                            "devices/%i/last_connected", row->device_id)) {
          return false;
        }
        cJSON *root = cJSON_CreateObject();
        if (!root) {
          free(state_topic);
          return false;
        }
        const auto unique_id = std::string("supla_") + row->user_suid +
                               "_" + object_id;
        cJSON_AddStringToObject(root, "uniq_id", unique_id.c_str());
        cJSON_AddStringToObject(root, "name", "Last connected");
        cJSON_AddStringToObject(root, "stat_t", state_topic);
        free(state_topic);
        cJSON_AddStringToObject(root, "device_class", "timestamp");
        cJSON_AddStringToObject(root, "entity_category", "diagnostic");

        cJSON *device = cJSON_AddObjectToObject(root, "device");
        if (!device) {
          cJSON_Delete(root);
          return false;
        }
        char manufacturer[20];
        get_mfr_name(row->device_mfr_id, manufacturer, sizeof(manufacturer));
        cJSON_AddStringToObject(device, "ids",
                                ("supla-iodevice-" + id).c_str());
        cJSON_AddStringToObject(device, "name", row->device_name);
        cJSON_AddStringToObject(device, "mf", manufacturer);
        cJSON_AddStringToObject(device, "sw", row->device_softver);
        char *payload = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        if (!payload) {
          return false;
        }
        *topic_name = strdup(discovery_topic.c_str());
        if (!*topic_name) {
          free(payload);
          return false;
        }
        *message = payload;
        if (message_size) {
          *message_size = strlen(payload);
        }
        return true;
      }
    }
  }

  return false;
}

void supla_mqtt_device_message_provider::set_data_row(
    _mqtt_db_data_row_device_t *row) {
  this->row = row;
  reset_index();
}
