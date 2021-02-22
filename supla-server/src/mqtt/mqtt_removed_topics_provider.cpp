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

#include <mqtt_removed_topics_provider.h>
#include <stdlib.h>
#include <string>
#include "user.h"

supla_mqtt_removed_topics_provider::supla_mqtt_removed_topics_provider(void)
    : supla_mqtt_message_provider() {}

supla_mqtt_removed_topics_provider::~supla_mqtt_removed_topics_provider(void) {
  clear();
}

bool supla_mqtt_removed_topics_provider::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (topic_name && topics.size()) {
    *topic_name = topics.front();
    topics.pop_front();
    return true;
  }

  return false;
}

void supla_mqtt_removed_topics_provider::clear(void) {
  for (std::list<char *>::iterator it = topics.begin(); it != topics.end();
       ++it) {
    free(*it);
  }
}

void supla_mqtt_removed_topics_provider::set_data(
    const char *topic_prefix, supla_mqtt_message_provider *before,
    supla_mqtt_message_provider *after) {
  clear();
  if (!before || !after) {
    return;
  }

  std::list<std::string> topics_after;
  char *topic_name = NULL;
  while (after->fetch(topic_prefix, &topic_name, NULL, NULL)) {
    if (topic_name) {
      topics_after.push_back(std::string(topic_name));
      free(topic_name);
      topic_name = NULL;
    }
  }

  while (before->fetch(topic_prefix, &topic_name, NULL, NULL)) {
    if (topic_name) {
      bool exists = false;
      for (std::list<std::string>::iterator it = topics_after.begin();
           it != topics_after.end(); ++it) {
        if (it->compare(topic_name) == 0) {
          exists = true;
          break;
        }
      }

      if (!exists) {
        topics.push_back(topic_name);
      } else {
        free(topic_name);
        topic_name = NULL;
      }
    }
  }
}
