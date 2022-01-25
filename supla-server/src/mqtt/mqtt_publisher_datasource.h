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

#ifndef MQTT_PUBLISHER_DATASOURCE_H_
#define MQTT_PUBLISHER_DATASOURCE_H_

#include <list>

#include "mqtt_action_message_provider.h"
#include "mqtt_client_db_datasource.h"
#include "mqtt_state_message_provider.h"

typedef struct {
  int user_id;
  int device_id;
  int channel_id;
  unsigned int actions;
  struct timeval time;
} _mqtt_ds_triggered_actions_t;

class supla_mqtt_publisher_datasource : public supla_mqtt_client_db_datasource {
 private:
  std::list<int> users_enabled;
  std::list<int> users_enabled_tmp;
  std::list<_mqtt_ds_triggered_actions_t> triggered_actions;

  bool fetch_users;
  bool fetch_devices;
  bool fetch_channels;
  bool fetch_states;

  void *user_query;
  void *device_query;
  void *channel_query;

  void *userdata_row;
  void *devicedata_row;
  void *channeldata_row;

  supla_mqtt_message_provider *user_message_provider;
  supla_mqtt_message_provider *device_message_provider;
  supla_mqtt_message_provider *channelandstate_message_provider;
  supla_mqtt_state_message_provider *state_message_provider;
  supla_mqtt_action_message_provider *action_message_provider;

  void add_user_to_list(int user_id, std::list<int> *ulist);
  bool is_user_enabled(int user_id);
  void *datarow_malloc(int datatype);
  void *open_query(int datatype, supla_mqtt_ds_context *context,
                   void *data_row);
  bool db_fetch_row(int datatype, void *query, void *data_row);
  supla_mqtt_message_provider *new_provider(int datatype, void *data_row);
  void set_provider_data_row(supla_mqtt_ds_context *context, int datatype,
                             supla_mqtt_message_provider *provider,
                             void *data_row);

  bool fetch(int datatype, void **query, void **data_row,
             supla_mqtt_message_provider **provider,
             supla_mqtt_ds_context *context, char **topic_name, void **message,
             size_t *message_size);
  bool fetch_actions(char **topic_name, void **message, size_t *message_size);
  bool fetch_user(supla_mqtt_ds_context *context, char **topic_name,
                  void **message, size_t *message_size);
  bool fetch_device(supla_mqtt_ds_context *context, char **topic_name,
                    void **message, size_t *message_size);
  bool fetch_channel(supla_mqtt_ds_context *context, char **topic_name,
                     void **message, size_t *message_size);
  bool fetch_state(supla_mqtt_ds_context *context, char **topic_name,
                   void **message, size_t *message_size);

  void close_userquery();
  void close_devicequery();
  void close_channelquery();

 protected:
  virtual bool is_context_allowed(supla_mqtt_ds_context *context);
  virtual bool context_open(supla_mqtt_ds_context *context);
  virtual bool _fetch(supla_mqtt_ds_context *context, char **topic_name,
                      void **message, size_t *message_size, bool *retain);
  virtual void context_close(supla_mqtt_ds_context *context);

 public:
  explicit supla_mqtt_publisher_datasource(
      supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_publisher_datasource(void);
  virtual bool fetch(char **topic_name, void **message, size_t *message_size,
                     bool *retain);
  void on_actions_triggered(int user_id, int device_id, int channel_id,
                            unsigned int actions);
};

#endif /*MQTT_PUBLISHER_DATASOURCE_H_*/
