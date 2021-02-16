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

#include "mqtt_subscriber.h"

supla_mqtt_subscriber::supla_mqtt_subscriber(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource,
    supla_mqtt_abstract_channel_value_setter *setter)
    : supla_mqtt_client(library_adapter, settings, datasource) {
  this->setter = setter;
}

ssize_t supla_mqtt_subscriber::get_send_buffer_size(void) { return 8192; }

ssize_t supla_mqtt_subscriber::get_recv_buffer_size(void) { return 262144; }

void supla_mqtt_subscriber::get_client_id(char *clientId, size_t len) {
  if (settings) {
    settings->getClientId(clientId, len, "sub");
  }
}

bool supla_mqtt_subscriber::on_iterate(void) {
  char *topic_name = NULL;
  bool result = false;

  if (datasource->fetch(&topic_name, NULL, NULL)) {
    subscribe(topic_name, SUPLA_MQTT_QOS_0);
    result = true;
  }

  if (topic_name) {
    free(topic_name);
  }

  return result;
}

void supla_mqtt_subscriber::on_message_received(
    const _received_mqtt_message_t *msg) {
  setter->set_value((char *)msg->topic_name, msg->topic_name_size,
                    (char *)msg->message, msg->message_size);
}
