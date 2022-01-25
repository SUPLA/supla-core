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

#include "mqtt_publisher.h"

#include <string.h>

#include <cstdio>

#include "log.h"
#include "mqtt_publisher_datasource.h"

supla_mqtt_publisher::supla_mqtt_publisher(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource)
    : supla_mqtt_client(library_adapter, settings, datasource) {}

supla_mqtt_publisher::~supla_mqtt_publisher(void) {}

ssize_t supla_mqtt_publisher::get_send_buffer_size(void) { return 32768; }

ssize_t supla_mqtt_publisher::get_recv_buffer_size(void) { return 32768; }

void supla_mqtt_publisher::get_client_id(char *clientId, size_t len) {
  if (settings) {
    settings->getClientId(clientId, len, "pub");
  }
}

bool supla_mqtt_publisher::on_iterate(void) {
  char *topic_name = NULL;
  void *message = NULL;
  size_t message_size = 0;
  bool result = false;
  bool retain = true;

  if (datasource->fetch(&topic_name, &message, &message_size, &retain)) {
    publish(topic_name, message, message_size, SUPLA_MQTT_QOS_0, retain);
    result = true;
  }

  if (topic_name) {
    free(topic_name);
  }

  if (message) {
    free(message);
  }

  return result;
}

void supla_mqtt_publisher::on_message_received(
    const _received_mqtt_message_t *msg) {}

void supla_mqtt_publisher::on_actions_triggered(int user_id, int device_id,
                                               int channel_id,
                                               unsigned int actions) {
  supla_mqtt_publisher_datasource *pds =
      dynamic_cast<supla_mqtt_publisher_datasource *>(datasource);
  if (pds) {
    pds->on_actions_triggered(user_id, device_id, channel_id, actions);
    library_adapter->raise_event();
  }
}
