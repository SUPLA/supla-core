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
#include "database.h"
#include "mqtt_client_library_adapter.h"
#include "mqtt_client_settings.h"
#include "mqtt_ds_context.h"

typedef struct {
  int user_id;
  int device_id;
} _mqtt_ds_device_id_t;

typedef struct {
  int user_id;
  int device_id;
  int channel_id;
} _mqtt_ds_channel_id_t;

class supla_mqtt_client_datasource {
 private:
  void *lck;
  bool _context_open;
  supla_mqtt_client_settings *settings = NULL;

  supla_mqtt_ds_context context;

  std::list<int> users;
  std::list<int> users_tmp;

  bool all_data_expected;
  std::list<int> user_queue;
  std::list<_mqtt_ds_device_id_t> device_queue;
  std::list<_mqtt_ds_channel_id_t> channel_queue;

  bool is_user_queued(int user_id);
  bool is_device_queued(int device_id);
  bool is_channel_queued(int channel_id);

  bool context_should_be_opened(void);
  void context_open(void);
  void context_close(void);
  void add_user_to_list(int user_id, std::list<int> *ulist);
  void update_user_list();
  bool is_mqtt_enabled(int user_id);

 protected:
  virtual bool is_scope_allowed(MQTTDataSourceScope scope) = 0;
  supla_mqtt_client_settings *get_settings(void);
  virtual bool context_open(supla_mqtt_ds_context *context) = 0;
  virtual bool _fetch(supla_mqtt_ds_context *context, char **topic_name,
                      void **message, size_t *message_size) = 0;
  virtual void context_close(supla_mqtt_ds_context *context) = 0;

 public:
  explicit supla_mqtt_client_datasource(supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_client_datasource(void);
  virtual void thread_init(void);
  virtual void thread_cleanup(void);
  bool fetch(char **topic_name, void **message, size_t *message_size);
  bool fetch(char **topic_name);

  virtual void on_broker_connected(void);
  virtual void on_userdata_changed(int user_id);
  virtual void on_devicedata_changed(int user_id, int device_id);
  virtual void on_channelstate_changed(int user_id, int device_id,
                                       int channel_id);

  bool is_context_open(void);
};

#endif /*MQTT_CLIENT_DATASOURCE_H_*/
