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

#include "mqtt_unpublisher.h"
#include <string.h>
#include <cstdio>
#include "lck.h"
#include "log.h"
#include "mqtt_db.h"

#define SUBSCRIPTION_TIME_SEC 5

supla_mqtt_unpublisher::supla_mqtt_unpublisher(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_unpublisher_datasource *datasource)
    : supla_mqtt_client(library_adapter, settings, datasource) {
  ds = datasource;
}

supla_mqtt_unpublisher::~supla_mqtt_unpublisher(void) {}

ssize_t supla_mqtt_unpublisher::get_send_buffer_size(void) { return 32768; }

ssize_t supla_mqtt_unpublisher::get_recv_buffer_size(void) { return 32768; }

void supla_mqtt_unpublisher::get_client_id(char *clientId, size_t len) {
  if (settings) {
    settings->getClientId(clientId, len, "unpub");
  }
}

bool supla_mqtt_unpublisher::on_iterate(void) {
  char *topic_name = NULL;

  bool result = false;

  if (ds->fetch(&topic_name, NULL, NULL)) {
    publish(topic_name, NULL, 0, SUPLA_MQTT_QOS_0, true);
    result = true;
  }

  if (topic_name) {
    free(topic_name);
    topic_name = NULL;
  }

  if (result) {
    return true;
  }

  bool _unsubscribe = false;
  if (ds->fetch_subscription(&topic_name, &_unsubscribe)) {
    if (_unsubscribe) {
      unsubscribe(topic_name);
    } else {
      subscribe(topic_name, SUPLA_MQTT_QOS_0);
    }
    result = true;
  }

  if (topic_name) {
    free(topic_name);
    topic_name = NULL;
  }

  return result;
}

void supla_mqtt_unpublisher::on_message_received(
    const _received_mqtt_message_t *msg) {
  if (msg->topic_name_size && msg->topic_name && msg->message_size &&
      msg->message) {
    char *topic_name = (char *)malloc(msg->topic_name_size + 1);
    memcpy(topic_name, msg->topic_name, msg->topic_name_size);
    topic_name[msg->topic_name_size] = 0;
    publish(topic_name, NULL, 0, SUPLA_MQTT_QOS_0, true);
  }
}
