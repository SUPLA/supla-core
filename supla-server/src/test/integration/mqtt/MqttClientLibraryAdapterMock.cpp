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
#include <unistd.h>
#include "lck.h"

MqttClientLibraryAdapterMock::MqttClientLibraryAdapterMock(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_library_adapter(settings) {
  this->lck = lck_init();
  this->connected = false;
}

MqttClientLibraryAdapterMock::~MqttClientLibraryAdapterMock(void) {
  lck_free(lck);
}

void MqttClientLibraryAdapterMock::client_connect(
    supla_mqtt_client *supla_client_instance) {
  lck_lock(lck);
  connected = true;
  lck_unlock(lck);
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
  return true;
}

bool MqttClientLibraryAdapterMock::publish(const char *topic_name,
                                           const void *message,
                                           size_t message_size,
                                           QOS_Level qos_level, bool retain) {
  return true;
}
