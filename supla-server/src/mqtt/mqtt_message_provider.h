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

#ifndef MQTT_TOPIC_PROVIDER_H_
#define MQTT_TOPIC_PROVIDER_H_

#include "database.h"
#include "mqtt_client_settings.h"

class supla_mqtt_message_provider {
 private:
  supla_mqtt_client_settings *settings;
  bool include_user_info;
  int offset;

 protected:
 public:
  supla_mqtt_message_provider(supla_mqtt_client_settings *settings);
  virtual ~supla_mqtt_message_provider(void);
  bool fetch(char **topic_name, void **message, size_t *message_size);
};

#endif /*MQTT_TOPIC_PROVIDER_H_*/
