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

#ifndef MQTT_CLIENT_LIBRARY_ADAPTER_INTERFACE_H_
#define MQTT_CLIENT_LIBRARY_ADAPTER_INTERFACE_H_

#include <stdlib.h>
#include "mqtt_client_settings.h"

class supla_mqtt_client;

enum QOS_Level { SUPLA_MQTT_QOS_0, SUPLA_MQTT_QOS_1, SUPLA_MQTT_QOS_2 };

typedef struct {
  unsigned short packet_id;

  bool re_delivery;
  QOS_Level qos_level;
  bool retain;
  unsigned short topic_name_size;
  const void *topic_name;
  size_t message_size;
  const void *message;
} _received_mqtt_message_t;

typedef void (*_on_message_received_cb)(supla_mqtt_client *client_instance,
                                        const _received_mqtt_message_t *msg);
typedef void (*_on_connected_cb)(supla_mqtt_client *client_instance);

class supla_mqtt_client_library_adapter {
 protected:
  supla_mqtt_client_settings *settings;
  _on_message_received_cb on_message_received_callback;
  _on_connected_cb on_connected_callback;

 public:
  explicit supla_mqtt_client_library_adapter(
      supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_client_library_adapter(void);
  virtual void client_connect(supla_mqtt_client *supla_client_instance) = 0;
  virtual bool is_connected(void) = 0;
  virtual void iterate(void) = 0;
  virtual void disconnect(void) = 0;
  virtual void cleanup(void) = 0;
  virtual void raise_event(void) = 0;
  virtual bool subscribe(const char *topic_name, QOS_Level max_qos_level) = 0;
  virtual bool unsubscribe(const char *topic_name) = 0;
  virtual bool publish(const char *topic_name, const void *message,
                       size_t message_size, QOS_Level qos_level,
                       bool retain) = 0;

  virtual void set_on_message_received_callback(_on_message_received_cb cb);
  virtual void set_on_connected_callback(_on_connected_cb cb);
};

#endif /*MQTT_CLIENT_LIBRARY_ADAPTER_INTERFACE_H_*/
