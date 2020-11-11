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

#include "mqtt_client.h"
#include <unistd.h>
#include "log.h"
#include "sthread.h"
#include "supla-socket.h"

// static
void supla_mqtt_client::job(void *supla_client_instance, void *sthread) {
  supla_mqtt_client *client =
      static_cast<supla_mqtt_client *>(supla_client_instance);
  if (client) {
    client->job(sthread);
  }
}

supla_mqtt_client::supla_mqtt_client(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource) {
  this->library_adapter = library_adapter;
  this->settings = settings;
  this->datasource = datasource;
  this->sthread = NULL;
  this->unable_to_connect_notified = false;
}

supla_mqtt_client::~supla_mqtt_client(void) {}

void supla_mqtt_client::job(void *sthread) {
  this->sthread = sthread;

  library_adapter->connect();

  while (!sthread_isterminated(sthread)) {
    library_adapter->iterate();

    if (library_adapter->is_connected()) {
      on_iterate();
    }
  }

  library_adapter->disconnect();
  library_adapter->cleanup();
}

void supla_mqtt_client::on_message_received(
    struct mqtt_response_publish *message) {}

void supla_mqtt_client::on_connected(void) {}

void supla_mqtt_client::on_iterate(void) {}

bool supla_mqtt_client::subscribe(const char *topic_name,
                                  SuplaMQTTFlags max_qos_level) {
  return !sthread_isterminated(sthread) &&
         library_adapter->subscribe(topic_name, max_qos_level);
}

bool supla_mqtt_client::publish(const char *topic_name, const void *message,
                                size_t message_size,
                                SuplaMQTTFlags publish_flags) {
  return !sthread_isterminated(sthread) &&
         library_adapter->publish(topic_name, message, message_size,
                                  publish_flags);
}

void supla_mqtt_client::start(void) {
  if (sthread == NULL && settings && settings->isMQTTEnabled()) {
    sthread = sthread_simple_run(supla_mqtt_client::job, this, 0);
  }
}

void supla_mqtt_client::stop(void) {
  if (sthread != NULL) {
    sthread_twf(sthread);
    sthread = NULL;
  }
}
