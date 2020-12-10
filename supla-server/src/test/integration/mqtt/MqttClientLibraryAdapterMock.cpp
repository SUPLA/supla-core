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

#include "MqttClientLibraryAdapterMock.h"
#include <string.h>
#include <unistd.h>
#include "lck.h"
#include "log.h"

MqttClientLibraryAdapterMock::MqttClientLibraryAdapterMock(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_library_adapter(settings) {
  this->lck = lck_init();
  this->connected = false;
}

MqttClientLibraryAdapterMock::~MqttClientLibraryAdapterMock(void) {
  published_clear();
  subscribed_clear();
  lck_free(lck);
}

void MqttClientLibraryAdapterMock::client_connect(
    supla_mqtt_client *supla_client_instance) {
  lck_lock(lck);
  this->supla_client_instance = supla_client_instance;
  connected = true;
  lck_unlock(lck);

  if (on_connected_callback) {
    on_connected_callback(supla_client_instance);
  }
}

bool MqttClientLibraryAdapterMock::is_connected(void) {
  bool result = false;

  lck_lock(lck);
  result = connected;
  lck_unlock(lck);

  return result;
}

void MqttClientLibraryAdapterMock::iterate(void) { usleep(100); }

void MqttClientLibraryAdapterMock::disconnect(void) {
  lck_lock(lck);
  connected = false;
  lck_unlock(lck);
}

void MqttClientLibraryAdapterMock::cleanup(void) {}

void MqttClientLibraryAdapterMock::raise_event(void) {}

bool MqttClientLibraryAdapterMock::subscribe(const char *topic_name,
                                             QOS_Level max_qos_level) {
  subscribed_messages.push_back(std::string(topic_name));
  return true;
}

bool MqttClientLibraryAdapterMock::publish(const char *topic_name,
                                           const void *message,
                                           size_t message_size,
                                           QOS_Level qos_level, bool retain) {
  _mqtt_test_message_t m;
  memset(&m, 0, sizeof(_mqtt_test_message_t));

  m.topic_name = strdup(topic_name);
  if (message && message_size) {
    m.message = (char *)malloc(message_size + 1);
    if (m.message) {
      memcpy(m.message, message, message_size);
      ((char *)m.message)[message_size] = 0;
      m.message_size = message_size;
    }
  }

  published_messages.push_back(m);
  return true;
}

void MqttClientLibraryAdapterMock::published_clear(void) {
  lck_lock(lck);
  while (published_messages.size()) {
    _mqtt_test_message_t m = published_messages.front();
    if (m.topic_name != NULL) {
      free(m.topic_name);
    }
    if (m.message) {
      free(m.message);
    }
    published_messages.pop_front();
  }
  lck_unlock(lck);
}

int MqttClientLibraryAdapterMock::published_count(void) {
  int result = 0;
  lck_lock(lck);
  result = published_messages.size();
  lck_unlock(lck);
  return result;
}

_mqtt_test_message_t MqttClientLibraryAdapterMock::published_pop(void) {
  _mqtt_test_message_t result;
  lck_lock(lck);
  result = published_messages.front();
  published_messages.pop_front();
  lck_unlock(lck);
  return result;
}

void MqttClientLibraryAdapterMock::subscribed_clear(void) {
  lck_lock(lck);
  subscribed_messages.clear();
  lck_unlock(lck);
}

int MqttClientLibraryAdapterMock::subscribed_count(void) {
  int result = 0;
  lck_lock(lck);
  result = subscribed_messages.size();
  lck_unlock(lck);
  return result;
}

std::string MqttClientLibraryAdapterMock::subscribed_pop(void) {
  std::string result;
  lck_lock(lck);
  result = subscribed_messages.front();
  subscribed_messages.pop_front();
  lck_unlock(lck);
  return result;
}

void MqttClientLibraryAdapterMock::on_message_received(const char *topic_name,
                                                       void *message,
                                                       size_t *message_size) {
  if (on_message_received_callback) {
    _received_mqtt_message_t msg;
    memset(&msg, 0, sizeof(_received_mqtt_message_t));

    msg.topic_name = topic_name;
    msg.message = message;
    msg.message_size = message_size ? *message_size : 0;

    on_message_received_callback(supla_client_instance, &msg);
  }
}
