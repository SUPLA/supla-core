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

#ifndef MQTT_DS_CONTEXT_H_
#define MQTT_DS_CONTEXT_H_

enum MQTTDataSourceScope {
  MQTTDS_SCOPE_NONE,
  MQTTDS_SCOPE_FULL,
  MQTTDS_SCOPE_USER,
  MQTTDS_SCOPE_DEVICE,
  MQTTDS_SCOPE_CHANNEL_STATE,
};

class supla_mqtt_ds_context {
 private:
  int user_id;
  int device_id;
  int channel_id;
  MQTTDataSourceScope scope;

 public:
  supla_mqtt_ds_context(MQTTDataSourceScope scope);
  supla_mqtt_ds_context(MQTTDataSourceScope scope, int user_id);
  supla_mqtt_ds_context(MQTTDataSourceScope scope, int user_id, int device_id);
  supla_mqtt_ds_context(MQTTDataSourceScope scope, int user_id, int device_id,
                        int channel_id);
  void set_user_id(int user_id);
  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);
  MQTTDataSourceScope get_scope(void);
};

#endif /*MQTT_DS_CONTEXT_H_*/
