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

#include "mqtt_abstract_channel_value_setter.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

supla_mqtt_abstract_channel_value_setter::
    supla_mqtt_abstract_channel_value_setter(
        supla_mqtt_client_settings *settings) {
  this->settings = settings;
}

supla_mqtt_abstract_channel_value_setter::
    ~supla_mqtt_abstract_channel_value_setter(void) {}

void supla_mqtt_abstract_channel_value_setter::set_value(const char *topic_name,
                                                         size_t topic_name_size,
                                                         const void *message,
                                                         size_t message_size) {
  if (topic_name == NULL || topic_name_size == 0 || message == NULL ||
      message_size == 0) {
    return;
  }

  size_t prefix_len =
      settings->getPrefix() ? strnlen(settings->getPrefix(), 512) : 0;

  if (prefix_len) {
    if (topic_name_size <= prefix_len) {
      return;
    }

    if (memcmp(topic_name, settings->getPrefix(), prefix_len) != 0) {
      return;
    }

    topic_name += prefix_len;
    topic_name_size -= prefix_len;

    if (memcmp(topic_name, "/", 1) != 0) {
      return;
    }

    topic_name++;
    topic_name_size--;
  }

  if (memcmp(topic_name, "supla/", 6) != 0) {
    return;
  }

  topic_name += 6;
  topic_name_size -= 6;

  size_t email_len = 0;
  char *email_ptr = NULL;

  bool at = false;
  size_t a = 0;

  for (a = 0; a < topic_name_size; a++) {
    if (!at && topic_name[a] == '@') {
      at = true;
    } else if (topic_name[a] == '/') {
      if (at) {
        email_len = a;
        email_ptr = (char *)topic_name;
        break;
      } else {
        return;
      }
    }
  }

  topic_name += email_len + 1;
  topic_name_size -= email_len + 1;

  if (memcmp(topic_name, "channels/", 9) != 0) {
    return;
  }

  topic_name += 9;
  topic_name_size -= 9;
  int channel_id = 0;

  for (a = 0; a < topic_name_size; a++) {
    if (topic_name[a] == '/') {
      if (a == 0) {
        return;
      }

      size_t b = a - 1;
      while (1) {
        channel_id += (topic_name[b] - 48) * pow(10, a - 1 - b);
        if (b == 0) {
          break;
        }
        b--;
      }

      topic_name += a + 1;
      topic_name_size -= a + 1;
      break;

    } else if (topic_name[a] < '0' || topic_name[a] > '9') {
      return;
    }
  }

  if (channel_id <= 0) {
    return;
  }

  if (topic_name_size == 6 && memcmp(topic_name, "set/on", 6) == 0) {
  };
}
