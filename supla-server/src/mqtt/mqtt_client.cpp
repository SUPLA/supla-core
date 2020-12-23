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
#include "log.h"
#include "sthread.h"

// static
void supla_mqtt_client::job(void *supla_client_instance, void *sthread) {
  supla_mqtt_client *client =
      static_cast<supla_mqtt_client *>(supla_client_instance);
  if (client) {
    client->job(sthread);
  }
}

// static
void supla_mqtt_client::on_connected(supla_mqtt_client *client_instance) {
  client_instance->on_connected();
}

// static
void supla_mqtt_client::on_message_received(
    supla_mqtt_client *client_instance, const _received_mqtt_message_t *msg) {
  client_instance->on_message_received(msg);
}

supla_mqtt_client::supla_mqtt_client(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource) {
  this->library_adapter = library_adapter;
  this->settings = settings;
  this->datasource = datasource;
  this->sthread = NULL;
}

supla_mqtt_client::~supla_mqtt_client(void) {}

void supla_mqtt_client::job(void *sthread) {
  this->sthread = sthread;

  datasource->thread_init();
  library_adapter->set_on_message_received_callback(
      supla_mqtt_client::on_message_received);
  library_adapter->set_on_connected_callback(supla_mqtt_client::on_connected);

  library_adapter->client_connect(this);

  while (!sthread_isterminated(sthread)) {
    library_adapter->iterate();

    if (library_adapter->is_connected()) {
      if (on_iterate()) {
        library_adapter->raise_event();
      }
    }
  }

  library_adapter->disconnect();
  library_adapter->cleanup();
  datasource->thread_cleanup();
}

void supla_mqtt_client::on_message_received(
    const _received_mqtt_message_t *msg) {}

void supla_mqtt_client::on_connected(void) {
  datasource->on_broker_connected();
}

bool supla_mqtt_client::on_iterate(void) {}

bool supla_mqtt_client::subscribe(const char *topic_name,
                                  QOS_Level max_qos_level) {
  return !sthread_isterminated(sthread) && library_adapter->is_connected() &&
         library_adapter->subscribe(topic_name, max_qos_level);
}

bool supla_mqtt_client::publish(const char *topic_name, const void *message,
                                size_t message_size, QOS_Level qos_level,
                                bool retain) {
  return !sthread_isterminated(sthread) && library_adapter->is_connected() &&
         library_adapter->publish(topic_name, message, message_size, qos_level,
                                  retain);
}

void supla_mqtt_client::start(void) {
  if (sthread == NULL && settings && settings->isMQTTEnabled()) {
    sthread = sthread_simple_run(supla_mqtt_client::job, this, 0);
  }
}

void supla_mqtt_client::stop(void) {
  if (sthread != NULL) {
    sthread_terminate(sthread);
    library_adapter->raise_event();
    sthread_twf(sthread);
    sthread = NULL;
  }
}

bool supla_mqtt_client::is_terminated(void) {
  return sthread == NULL || sthread_isterminated(sthread);
}

void supla_mqtt_client::on_userdata_changed(int user_id) {
  datasource->on_userdata_changed(user_id);
  library_adapter->raise_event();
}

void supla_mqtt_client::on_devicedata_changed(int user_id, int device_id) {
  datasource->on_devicedata_changed(user_id, device_id);
  library_adapter->raise_event();
}

void supla_mqtt_client::on_channelstate_changed(int user_id, int device_id,
                                                int channel_id) {
  datasource->on_channelstate_changed(user_id, device_id, channel_id);
  library_adapter->raise_event();
}
