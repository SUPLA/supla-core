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
#include <stdlib.h>
#include <string.h>

#define MQTT_MAX_TOPIC_NAME_SIZE 32767
#define MQTT_MAX_MESSAGE_SIZE 1048576

supla_mqtt_message_provider::supla_mqtt_message_provider(void) {
  this->message_count = 0;
  this->index = 0;
}

supla_mqtt_message_provider::~supla_mqtt_message_provider(void) {}

bool supla_mqtt_message_provider::create_message(
    const char *topic_prefix, const char *email, char **topic_name_out,
    void **message, size_t *message_size, const char *topic_name_in,
    const char *message_string_in, bool include_null_byte) {
  if (topic_name_in == NULL || topic_name_out == NULL) {
    return false;
  }
  size_t tn_size = strnlen(topic_name_in, MQTT_MAX_TOPIC_NAME_SIZE);

  if (tn_size == 0) {
    return false;
  }

  size_t prefix_len = 0;
  size_t email_len = 0;

  if (topic_prefix &&
      (prefix_len = strnlen(topic_prefix, MQTT_MAX_TOPIC_NAME_SIZE)) > 0) {
    if (strncmp(topic_prefix, "/%email%", MQTT_MAX_TOPIC_NAME_SIZE) == 0) {
      if (email && (email_len = strnlen(email, SUPLA_EMAIL_MAXSIZE)) > 0) {
        prefix_len = email_len + 1;
      } else {
        prefix_len = 0;
        email_len = 0;
      }
    }
  }

  tn_size += prefix_len + 1;
  *topic_name_out = (char *)malloc(tn_size);
  if (*topic_name_out) {
    snprintf(*topic_name_out, tn_size, "%s%s%s", email_len ? "/" : "",
             email_len ? email : (prefix_len ? topic_prefix : ""),
             topic_name_in);

    if (message && message_string_in) {
      size_t size = strnlen(message_string_in, MQTT_MAX_MESSAGE_SIZE);
      if (include_null_byte) {
        size += 1;
      }
      *message = malloc(size);
      if (*message) {
        memcpy(*message, message_string_in, size);

        if (message_size) {
          *message_size = size;
        }
      }
    }

    return true;
  }

  return false;
}

void supla_mqtt_message_provider::set_message_count(
    unsigned short message_count) {
  this->message_count = message_count;
  index = 0;
}

bool supla_mqtt_message_provider::fetch(const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size) {
  bool result = false;
  if (index < message_count && topic_name) {
    result = get_message_at_index(index, topic_prefix, topic_name, message,
                                  message_size) &&
             ((topic_name && *topic_name) || (message && *message));
    index++;
  }

  return result;
}
