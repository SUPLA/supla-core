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
        return create_message(topic_prefix, row->user_suid, topic_name,
                              message, message_size,
                              row->device_enabled ? "true" : "false", false,
                              "devices/%i/enabled", row->device_id);
      case 1:
        return create_message(topic_prefix, row->user_suid, topic_name,
                              message, message_size, row->device_last_connected,
                              false, "devices/%i/last_connected",
                              row->device_id);
      case 2:
        return create_message(topic_prefix, row->user_suid, topic_name,
                              message, message_size, row->device_last_ipv4,
                              false, "devices/%i/last_ipv4", row->device_id);
      case 3: {
        char mfr_name[20];
        get_mfr_name(row->device_mfr_id, mfr_name, sizeof(mfr_name));
        return create_message(topic_prefix, row->user_suid, topic_name,
                              message, message_size, mfr_name, false,
                              "devices/%i/manufacturer", row->device_id);
      }

      case 4:
        return create_message(topic_prefix, row->user_suid, topic_name,
                              message, message_size, row->device_name, false,
                              "devices/%i/name", row->device_id);
      case 5: {
        char proto_ver[15];
        snprintf(proto_ver, sizeof(proto_ver), "%i", row->device_proto_version);

        return create_message(topic_prefix, row->user_suid, topic_name,
                              message, message_size, proto_ver, false,
                              "devices/%i/proto_ver", row->device_id);
      }
      case 6:
        return create_message(topic_prefix, row->user_suid, topic_name,
                              message, message_size, row->device_softver, false,
                              "devices/%i/soft_ver", row->device_id);
    }
  }

  return false;
}

void supla_mqtt_device_message_provider::set_data_row(
    _mqtt_db_data_row_device_t *row) {
  this->row = row;
  reset_index();
}
