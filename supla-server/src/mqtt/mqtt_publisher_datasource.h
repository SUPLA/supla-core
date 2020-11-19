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

#include "mqtt_client_db_datasource.h"

class supla_mqtt_publisher_datasource : public supla_mqtt_client_db_datasource {
 private:
  bool fetch_users;
  bool fetch_devices;
  bool fetch_channels;
  bool fetch_values;

  void *user_query;
  void *device_query;
  void *channel_query;
  _mqtt_db_data_row_user_t *userdata_row;
  _mqtt_db_data_row_device_t *devicedata_row;
  _mqtt_db_data_row_channel_t *channeldata_row;

  bool fetch_user(const _mqtt_ds_context_t *context, char **topic_name,
                  void **message, size_t *message_size);
  bool fetch_device(const _mqtt_ds_context_t *context, char **topic_name,
                    void **message, size_t *message_size);
  bool fetch_channel(const _mqtt_ds_context_t *context, char **topic_name,
                     void **message, size_t *message_size);
  bool fetch_value(const _mqtt_ds_context_t *context, char **topic_name,
                   void **message, size_t *message_size);

  void close_userquery();
  void close_devicequery();
  void close_channelquery();

 protected:
  virtual bool context_open(const _mqtt_ds_context_t *context);
  virtual bool _fetch(const _mqtt_ds_context_t *context, char **topic_name,
                      void **message, size_t *message_size);
  virtual void context_close(const _mqtt_ds_context_t *context);

 public:
  supla_mqtt_publisher_datasource(void);
  virtual ~supla_mqtt_publisher_datasource(void);
};

#endif /*MQTT_CHANNEL_SOURCE_H_*/
