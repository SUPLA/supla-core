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

#ifndef MQTT_MESSAGE_PROVIDER_H_
#define MQTT_MESSAGE_PROVIDER_H_

#include "database.h"
#include "mqtt_client_settings.h"

class supla_mqtt_message_provider {
 private:
  unsigned short message_count;
  unsigned short index;

 protected:
  bool create_message(const char *topic_prefix, const char *email,
                      char **topic_name_out, void **message,
                      size_t *message_size, const char *message_string_in,
                      bool include_null_byte, const char *topic_name_in, ...);

  void set_message_count(unsigned short message_count);
  virtual bool get_message_at_index(unsigned short index,
                                    const char *topic_prefix, char **topic_name,
                                    void **message, size_t *message_size) = 0;

 public:
  supla_mqtt_message_provider(void);
  virtual ~supla_mqtt_message_provider(void);
  bool fetch(const char *topic_prefix, char **topic_name, void **message,
             size_t *message_size);
};

#endif /*MQTT_MESSAGE_PROVIDER_H_*/
