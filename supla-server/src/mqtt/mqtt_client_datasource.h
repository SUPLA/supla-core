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

#ifndef MQTT_CHANNEL_SOURCE_H_
#define MQTT_CHANNEL_SOURCE_H_

#include <stdlib.h>
#include "mqtt_client_library_adapter.h"

enum MQTTDataSourceEventType {
  MQTTDS_EVENT_RECONNECT,
  MQTTDS_EVENT_USER_DATACHANGED,
  MQTTDS_EVENT_DEVICEDATACHANGED,
  MQTTDS_EVENT_CHANNELDATACHANGED
};

class supla_mqtt_client_datasource {
 public:
  supla_mqtt_client_datasource(void);
  virtual ~supla_mqtt_client_datasource(void);
  virtual bool pop(char **topic_name, void **message, size_t *message_size,
                   QOS_Level *qos_level, bool *retain) = 0;
  virtual bool pop(char **topic_name, QOS_Level *qos_level) = 0;
  virtual void raise_event(MQTTDataSourceEventType event_type, int id = 0) = 0;
};

#endif /*MQTT_CHANNEL_SOURCE_H_*/
