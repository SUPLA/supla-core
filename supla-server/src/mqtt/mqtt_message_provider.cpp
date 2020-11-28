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
  memset(&args, 0, sizeof(va_list));

  va_start(args, topic_name_in);
  size_t tn_size = vsnprintf(NULL, 0, topic_name_in, args);
  va_end(args);

  if (tn_size == 0) {
    return false;
  }

  size_t email_len = 0;
  size_t prefix_len = 0;
  size_t offset = 0;
  bool result = false;

  if (topic_prefix &&
      (prefix_len = strnlen(topic_prefix, MQTT_MAX_TOPIC_NAME_SIZE)) > 0) {
    if (topic_prefix[prefix_len - 1] != '/') {
      tn_size++;
    }
    tn_size += prefix_len + 1;
    *topic_name_out = (char *)malloc(tn_size);
    if (email) {
      email_len = strnlen(email, SUPLA_EMAIL_MAXSIZE);
    }

    for (size_t a = 0; a < prefix_len; a++) {
      if (topic_prefix[a] == '%' && prefix_len - a >= 7 &&
          topic_prefix[a + 1] == 'e' && topic_prefix[a + 2] == 'm' &&
          topic_prefix[a + 3] == 'a' && topic_prefix[a + 4] == 'i' &&
          topic_prefix[a + 5] == 'l' && topic_prefix[a + 6] == '%') {
        if (email_len != 7) {
          tn_size += email_len - 7;
          *topic_name_out = (char *)realloc(*topic_name_out, tn_size);
        }
        if (email_len != 0) {
          memcpy(&(*topic_name_out)[offset], email, email_len);
        }
        a += 6;
        offset += email_len;
      } else {
        (*topic_name_out)[offset] = topic_prefix[a];
        offset++;
      }
    }

    if (offset > 0 && (*topic_name_out)[offset - 1] != '/') {
      (*topic_name_out)[offset] = '/';
      offset++;
    }

  } else {
    tn_size++;
    *topic_name_out = (char *)malloc(tn_size);
  }

  if (*topic_name_out) {
    va_start(args, topic_name_in);
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

void supla_mqtt_message_provider::reset_index(void) { index = 0; }

bool supla_mqtt_message_provider::fetch(const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size) {
  if (get_message_at_index(index, topic_prefix, topic_name, message,
                           message_size) &&
      ((topic_name && *topic_name) || (message && *message))) {
    index++;
    return true;
  }

  return false;
}
