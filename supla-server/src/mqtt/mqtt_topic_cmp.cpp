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

#include "mqtt_topic_cmp.h"
#include <string.h>

// static
void supla_mqtt_topic_cmp::load(const char *topic_prefix,
                                std::list<std::string> &topics,
                                supla_mqtt_message_provider *provider) {
  char *topic_name = NULL;
  provider->reset_index();
  while (provider->fetch(topic_prefix, &topic_name, NULL, NULL)) {
    if (topic_name != NULL) {
      if (strnlen(topic_name, 1) > 0) {
        topics.push_back(std::string(topic_name));
      }
      free(topic_name);
      topic_name = NULL;
    }
  }

  if (topic_name != NULL) {
    free(topic_name);
  }
}

// static
std::list<std::string> supla_mqtt_topic_cmp::topics_removed(
    const char *topic_prefix, supla_mqtt_message_provider *before,
    supla_mqtt_message_provider *after) {
  std::list<std::string> topics_before;
  std::list<std::string> topics_after;

  load(topic_prefix, topics_before, before);
  load(topic_prefix, topics_after, after);

  std::list<std::string>::iterator ita;
  std::list<std::string>::iterator itb;

  for (itb = topics_before.begin(); itb != topics_before.end(); ++itb) {
    for (ita = topics_after.begin(); ita != topics_after.end(); ++ita) {
      if (itb->compare(*ita) == 0) {
        itb = topics_before.erase(itb);
        --itb;
        break;
      }
    }
  }

  return topics_before;
}
