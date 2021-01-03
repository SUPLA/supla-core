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

#ifndef MQTT_UNPUBLISHER_DATASOURCE_H_
#define MQTT_UNPUBLISHER_DATASOURCE_H_

#include <list>
#include "mqtt_channelandstate_message_provider.h"
#include "mqtt_channelandstate_removed_topics_provider.h"
#include "mqtt_client_db_datasource.h"
#include "mqtt_device_message_provider.h"
#include "mqtt_unpublisher_user_topic_provider.h"

typedef struct {
  int user_id;
  bool mqtt_enabled;
  bool needs_subscribe;
  struct timeval subscribe_timeto;
} _unpub_user_item_t;

typedef struct {
  _mqtt_db_data_row_device_t device;
  std::list<_mqtt_db_data_row_channel_t> channels;
  struct timeval event_time;
} _unpub_device_item_t;

typedef struct {
  _mqtt_db_data_row_channel_t before;
  _mqtt_db_data_row_channel_t after;
  struct timeval event_time;
} _unpub_channel_item_t;

class supla_mqtt_unpublisher_datasource
    : public supla_mqtt_client_db_datasource {
 private:
  std::list<_unpub_user_item_t> users;
  supla_mqtt_unpublisher_user_topic_provider *user_topic_provider;
  _mqtt_db_data_row_user_t row;

  std::list<_unpub_device_item_t> deleted_devices;
  _unpub_device_item_t *current_device;
  supla_mqtt_device_message_provider *device_message_provider;
  supla_mqtt_channelandstate_message_provider *channel_message_provider;
  _mqtt_db_data_row_channel_t *current_channel_row;

  std::list<_unpub_channel_item_t> modified_channels;
  supla_mqtt_channelandstate_removed_topics_provider *removed_topics_provider;

  void remove_expired(void);
  bool fetch_deleted_device(char **topic_name);
  bool load_channel_row(int UserID, int ChannelID,
                        _mqtt_db_data_row_channel_t *row);

 protected:
  bool is_user_enabled(int user_id);
  virtual bool is_context_allowed(supla_mqtt_ds_context *context);
  virtual bool context_open(supla_mqtt_ds_context *context);
  virtual bool _fetch(supla_mqtt_ds_context *context, char **topic_name,
                      void **message, size_t *message_size);
  virtual void context_close(supla_mqtt_ds_context *context);

 public:
  explicit supla_mqtt_unpublisher_datasource(
      supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_unpublisher_datasource(void);
  virtual void thread_init(void);
  bool fetch_subscription(char **topic_name, bool *unsubscribe);
  virtual void on_userdata_changed(int user_id);
  virtual void before_channel_function_change(int UserID, int ChannelID);
  virtual void on_devicedata_changed(int UserID, int DeviceID);
  virtual void before_device_delete(int UserID, int DeviceID);
  virtual void on_device_deleted(int UserID, int DeviceID);
};

#endif /*MQTT_UNPUBLISHER_DATASOURCE_H_*/
