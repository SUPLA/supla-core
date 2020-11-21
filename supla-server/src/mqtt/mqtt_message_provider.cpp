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
#include <stdarg.h>
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
    void **message, size_t *message_size, const char *message_string_in,
    bool include_null_byte, const char *topic_name_in, ...) {
  if (topic_name_in == NULL || topic_name_out == NULL) {
    return false;
  }

  va_list args;
  va_start(args, topic_name_in);

  size_t tn_size = vsnprintf(NULL, 0, topic_name_in, args);

  if (tn_size == 0) {
    va_end(args);
    return false;
  }

  size_t prefix_len = 0;
  size_t email_len = 0;
  bool result = false;

  if (topic_prefix &&
      (prefix_len = strnlen(topic_prefix, MQTT_MAX_TOPIC_NAME_SIZE)) > 0 &&
      topic_prefix[0] == '/') {
    if (strncmp(topic_prefix, "/%email%", MQTT_MAX_TOPIC_NAME_SIZE) == 0) {
      if (email && (email_len = strnlen(email, SUPLA_EMAIL_MAXSIZE)) > 0) {
        prefix_len = email_len + 1;
      } else {
        prefix_len = 0;
        email_len = 0;
      }
    }
  } else {
    prefix_len = 0;
  }

  tn_size += prefix_len + 1;
  if (tn_size > MQTT_MAX_TOPIC_NAME_SIZE) {
    tn_size = MQTT_MAX_TOPIC_NAME_SIZE;
  }

  *topic_name_out = (char *)malloc(tn_size);

  if (*topic_name_out) {
    size_t offset = 0;

    if (email_len) {
      snprintf(*topic_name_out, tn_size, "/%s", email);
      offset = email_len + 1;
    } else if (prefix_len) {
      snprintf(*topic_name_out, tn_size, "%s", topic_prefix);
      offset = prefix_len;
    }

    vsnprintf(&(*topic_name_out)[offset], tn_size - offset, topic_name_in,
              args);
    va_end(args);

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

    result = true;
  }

  return result;
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
