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

#ifndef MQTT_CLIENT_H_
#define MQTT_CLIENT_H_

#include <mqtt_client_datasource.h>
#include <mqtt_client_library_adapter.h>
#include "mqtt_client_settings.h"

class supla_mqtt_client {
 private:
  void *sthread;

  static void job(void *supla_client_instance, void *sthread);
  void job(void *sthread);

  static void on_connected(supla_mqtt_client *client_instance);
  static void on_message_received(supla_mqtt_client *client_instance,
                                  const _received_mqtt_message_t *msg);

 protected:
  supla_mqtt_client_settings *settings;
  supla_mqtt_client_datasource *datasource;
  supla_mqtt_client_library_adapter *library_adapter;

  virtual void on_connected(void);
  virtual void on_message_received(const _received_mqtt_message_t *msg) = 0;
  virtual bool on_iterate(void) = 0;

  bool subscribe(const char *topic_name, QOS_Level max_qos_level);
  bool publish(const char *topic_name, const void *message, size_t message_size,
               QOS_Level qos_level, bool retain);

 public:
  supla_mqtt_client(supla_mqtt_client_library_adapter *library_adapter,
                    supla_mqtt_client_settings *settings,
                    supla_mqtt_client_datasource *datasource);
  virtual ~supla_mqtt_client(void);
  void start(void);
  void stop(void);
  bool is_terminated(void);

  virtual ssize_t get_send_buffer_size(void) = 0;
  virtual ssize_t get_recv_buffer_size(void) = 0;
  virtual void get_client_id(char *clientId, size_t len) = 0;

  virtual void on_userdata_changed(int user_id);
  virtual void on_devicedata_changed(int user_id, int device_id);
  virtual void on_channelstate_changed(int user_id, int device_id,
                                       int channel_id);
};

#endif /*MQTT_CLIENT_H_*/
