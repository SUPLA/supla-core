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

supla_mqtt_publisher::supla_mqtt_publisher(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *channel_source)
    : supla_mqtt_client(library_adapter, settings, channel_source) {}

supla_mqtt_publisher::~supla_mqtt_publisher(void) {}

ssize_t supla_mqtt_publisher::get_send_buffer_size(void) { return 32768; }

ssize_t supla_mqtt_publisher::get_recv_buffer_size(void) { return 32768; }

void supla_mqtt_publisher::get_client_id(char *clientId, size_t len) {
  if (settings) {
    settings->getClientId(clientId, len, "pub");
  }
}

void supla_mqtt_publisher::on_iterate(void) {
  char *topic_name = NULL;
  void *message = NULL;
  size_t message_size = 0;

  if (datasource->fetch(&topic_name, &message, &message_size)) {
    publish(topic_name, message, message_size, SUPLA_MQTT_QOS_0, true);
  }

  if (topic_name) {
    free(topic_name);
  }

  if (message) {
    free(message);
  }
}

void supla_mqtt_publisher::on_message_received(
    struct mqtt_response_publish *message) {}
