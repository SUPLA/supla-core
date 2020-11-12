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

#ifndef MQTTC_LIBRARY_ADAPTER_H_
#define MQTTC_LIBRARY_ADAPTER_H_

#include <mqtt_client_library_adapter.h>
#include "eh.h"
#include "mqtt.h"

class supla_mqttc_library_adapter;
typedef struct {
  supla_mqttc_library_adapter *instance;
  ssize_t (*__sendall)(supla_mqttc_library_adapter *adapter_instance,
                       const char *buf, size_t len, int flags);
  ssize_t (*__recvall)(supla_mqttc_library_adapter *adapter_instance, char *buf,
                       size_t bufsz, int flags);
} _sendrecv_methods_t;

class supla_mqttc_library_adapter : public supla_mqtt_client_library_adapter {
 private:
  _sendrecv_methods_t m;
  supla_mqtt_client *supla_client_instance;
  int sockfd;
  void *bio;
  void *ssl_ctx;
  struct mqtt_client client;
  TEventHandler *eh;
  bool unable_to_connect_notified;

  void *recvbuf;
  void *sendbuf;

  static ssize_t __mqtt_pal_sendall(
      supla_mqttc_library_adapter *adapter_instance, const char *buf,
      size_t len, int flags);
  static ssize_t __mqtt_pal_recvall(
      supla_mqttc_library_adapter *adapter_instance, char *buf, size_t bufsz,
      int flags);

  static void reconnect(struct mqtt_client *client,
                        void **supla_client_instance);
  static void on_message_received(void **supla_client_instance,
                                  struct mqtt_response_publish *message);
  void on_message_received(struct mqtt_response_publish *message);

  ssize_t mqtt_pal_sendall(const char *buf, size_t len, int flags);
  ssize_t mqtt_pal_recvall(char *buf, size_t bufsz, int flags);

  bool posix_connect(const char *port);
  void ssl_free(void);
  bool ssl_connect(const char *portd);
  void reconnect(struct mqtt_client *client);

 public:
  supla_mqttc_library_adapter(supla_mqtt_client_settings *settings);
  virtual ~supla_mqttc_library_adapter(void);
  virtual void client_connect(supla_mqtt_client *supla_client_instance);
  virtual bool is_connected(void);
  virtual void iterate(void);
  virtual void disconnect(void);
  virtual void cleanup(void);
  virtual void raise_event(void);

  virtual bool subscribe(const char *topic_name, QOS_Level max_qos_level);
  virtual bool publish(const char *topic_name, const void *message,
                       size_t message_size, QOS_Level qos_level, bool retain);
};

#endif /*MQTTC_LIBRARY_ADAPTER_H_*/
