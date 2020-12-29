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

supla_mqtt_message_provider::supla_mqtt_message_provider(void) {
  this->index = 0;
}

supla_mqtt_message_provider::~supla_mqtt_message_provider(void) {}

char *supla_mqtt_message_provider::homeassistant_get_node_id(
    const char *_email) {
  char *email = strndup(_email, SUPLA_EMAIL_MAXSIZE);
  if (email) {
    int email_len = strnlen(_email, SUPLA_EMAIL_MAXSIZE);
    for (int a = 0; a < email_len; a++) {
      char c = email[a];
      if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') &&
          (c < '0' || c > '9') && c != '_' && c != '-') {
        email[a] = '_';
      }
    }
    return email;
  }

  return NULL;
}

bool supla_mqtt_message_provider::create_message(
    const char *topic_prefix, const char *email, char **topic_name_out,
    void **message, size_t *message_size, const char *message_string_in,
    bool include_null_byte, const char *topic_name_in, ...) {
  size_t email_len = 0;
  size_t prefix_len = 0;

  if (topic_name_in == NULL || topic_name_out == NULL || email == NULL ||
      (email_len = strnlen(email, SUPLA_EMAIL_MAXSIZE)) == 0) {
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

  prefix_len =
      topic_prefix ? strnlen(topic_prefix, MQTT_MAX_TOPIC_NAME_SIZE) : 0;
  if (prefix_len > 0) {
    prefix_len++;
  }
  tn_size += prefix_len + email_len + 8;

  *topic_name_out = (char *)malloc(tn_size);

  if (*topic_name_out) {
    snprintf(*topic_name_out, tn_size, "%s%ssupla/%s/",
             prefix_len > 0 ? topic_prefix : "", prefix_len > 0 ? "/" : "",
             email);

    size_t offset = prefix_len + email_len + 7;

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

    return true;
  }

  return false;
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
