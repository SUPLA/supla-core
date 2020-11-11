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

enum SuplaMQTTFlags {
  SUPLA_MQTT_QOS_0,
  SUPLA_MQTT_QOS_1,
  SUPLA_MQTT_QOS_2,
  SUPLA_MQTT_RETAIN
};

class supla_mqtt_client_library_adapter {
 private:
  supla_mqtt_client_settings *settings;

 public:
  supla_mqtt_client_library_adapter(supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_client_library_adapter(void);
  virtual void connect(void) = 0;
  virtual bool is_connected(void) = 0;
  virtual void iterate(void) = 0;
  virtual void disconnect(void) = 0;
  virtual void cleanup(void) = 0;
  virtual bool subscribe(const char *topic_name,
                         SuplaMQTTFlags max_qos_level) = 0;
  virtual bool publish(const char *topic_name, const void *message,
                       size_t message_size, SuplaMQTTFlags publish_flags) = 0;
};

#endif /*MQTT_CLIENT_LIBRARY_ADAPTER_INTERFACE_H_*/
