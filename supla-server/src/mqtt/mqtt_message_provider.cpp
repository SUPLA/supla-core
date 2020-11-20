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

#include <mqtt_message_provider.h>

#define MP_OFFSET_BEGIN -1

supla_mqtt_message_provider::supla_mqtt_message_provider(void) {
  this->message_count = 0;
  this->index = 0;
}

supla_mqtt_message_provider::~supla_mqtt_message_provider(void) {}

void supla_mqtt_message_provider::set_message_count(
    unsigned short message_count) {
  this->message_count = message_count;
  index = 0;
}

bool supla_mqtt_message_provider::fetch(const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size) {
  bool result = false;
  if (index < message_count && (topic_name || message)) {
    result = get_message_at_index(index, topic_prefix, topic_name, message,
                                  message_size) &&
             ((topic_name && *topic_name) || (message && *message));
    index++;
  }

  return result;
}
