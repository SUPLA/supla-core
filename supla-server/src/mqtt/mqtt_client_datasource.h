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

#ifndef MQTT_CLIENT_DATASOURCE_H_
#define MQTT_CLIENT_DATASOURCE_H_

#include <stdlib.h>
#include <list>
#include "mqtt_client_library_adapter.h"

typedef struct {
  int user_id;
  int device_id;
} _mqtt_ds_device_id_t;

typedef struct {
  int user_id;
  int device_id;
  int channel_id;
} _mqtt_ds_channel_id_t;

enum MQTTDataSourceScope {
  MQTTDS_SCOPE_NONE,
  MQTTDS_SCOPE_FULL,
  MQTTDS_SCOPE_USER,
  MQTTDS_SCOPE_DEVICE,
  MQTTDS_SCOPE_CHANNEL_VALUE,
};

typedef struct {
  MQTTDataSourceScope scope;
  int user_id;
  int device_id;
  int channel_id;
} _mqtt_ds_context_t;

class supla_mqtt_client_datasource {
 private:
  void *lck;
  void *cursor;

  _mqtt_ds_context_t context;

  bool all_data_expected;
  std::list<int> user_queue;
  std::list<_mqtt_ds_device_id_t> device_queue;
  std::list<_mqtt_ds_channel_id_t> channel_queue;

  bool is_user_queued(int user_id);
  bool is_device_queued(int device_id);
  bool is_channel_queued(int channel_id);

  bool cursor_should_be_initialized(void);
  void reset_context(_mqtt_ds_context_t *scope);
  void cursor_release(void);

 protected:
  virtual void *cursor_init(const _mqtt_ds_context_t *context) = 0;
  virtual bool _pop(const _mqtt_ds_context_t *context, void *cursor,
                   char **topic_name, void **message, size_t *message_size,
                   bool *eof) = 0;
  virtual void cursor_release(const _mqtt_ds_context_t *context,
                              void *cursor) = 0;

 public:
  supla_mqtt_client_datasource(void);
  virtual ~supla_mqtt_client_datasource(void);
  bool pop(char **topic_name, void **message, size_t *message_size);
  bool pop(char **topic_name);

  void on_broker_connected(void);
  void on_userdata_changed(int user_id);
  void on_devicedata_changed(int user_id, int device_id);
  void on_channelvalue_changed(int user_id, int device_id, int channel_id);
};

#endif /*MQTT_CLIENT_DATASOURCE_H_*/
