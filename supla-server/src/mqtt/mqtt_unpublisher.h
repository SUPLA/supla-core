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

#ifndef MQTT_UNPUBLISHER_H_
#define MQTT_UNPUBLISHER_H_

#include <sys/time.h>
#include <list>
#include "mqtt_client.h"

typedef struct {
  int user_id;
  bool enabled;
  struct timeval time;
} _unpub_user_item_t;

class supla_mqtt_unpublisher : public supla_mqtt_client {
 private:
  std::list<_unpub_user_item_t> users;

 protected:
  virtual ssize_t get_send_buffer_size(void);
  virtual ssize_t get_recv_buffer_size(void);
  virtual void get_client_id(char *clientId, size_t len);
  virtual bool on_iterate(void);
  virtual void on_message_received(const _received_mqtt_message_t *msg);

 public:
  supla_mqtt_unpublisher(supla_mqtt_client_library_adapter *library_adapter,
                         supla_mqtt_client_settings *settings,
                         supla_mqtt_client_datasource *datasource);
  virtual ~supla_mqtt_unpublisher(void);
};

#endif /*MQTT_UNPUBLISHER_H_*/
