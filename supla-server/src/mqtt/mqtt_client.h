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

#include "eh.h"
#include "mqtt.h"
#include "mqtt_channel_source.h"
#include "mqtt_client_settings.h"

class supla_mqtt_client {
 private:
  int sockfd;
  void *sthread;
  void *sbio;
  TEventHandler *eh;
  static void job(void *supla_client_instance, void *sthread);
  static ssize_t __mqtt_pal_sendall(supla_mqtt_client *supla_client_instance,
                                    const char *buf, size_t len, int flags);
  static ssize_t __mqtt_pal_recvall(supla_mqtt_client *supla_client_instance,
                                    char *buf, size_t bufsz, int flags);

  static void reconnect(struct mqtt_client *client,
                        void **supla_client_instance);
  static void on_message_received(void **supla_client_instance,
                                  struct mqtt_response_publish *message);

  void job(void *sthread);
  ssize_t mqtt_pal_sendall(const char *buf, size_t len, int flags);
  ssize_t mqtt_pal_recvall(char *buf, size_t bufsz, int flags);

  void connect(void);
  void disconnect(void);
  void reconnect(struct mqtt_client *client);
  void on_message_received(struct mqtt_response_publish *message);

 protected:
  supla_mqtt_client_settings *settings;
  supla_mqtt_client_datasource *datasource;

 public:
  supla_mqtt_client(supla_mqtt_client_settings *settings,
                    supla_mqtt_client_datasource *datasource);
  virtual ~supla_mqtt_client(void);
  void start(void);
  void stop(void);
};

#endif /*MQTT_CLIENT_H_*/
