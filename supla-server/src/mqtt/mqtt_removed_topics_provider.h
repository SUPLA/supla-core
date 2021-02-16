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

#ifndef MQTT_CHANNEL_REMOVED_TOPICS_PROVIDER_H_
#define MQTT_CHANNEL_REMOVED_TOPICS_PROVIDER_H_

#include <list>
#include "mqtt_message_provider.h"

class supla_mqtt_removed_topics_provider : public supla_mqtt_message_provider {
 private:
  std::list<char *> topics;
  void clear(void);

 public:
  supla_mqtt_removed_topics_provider(void);
  virtual ~supla_mqtt_removed_topics_provider(void);
  virtual bool get_message_at_index(unsigned short index,
                                    const char *topic_prefix, char **topic_name,
                                    void **message, size_t *message_size);
  void set_data(const char *topic_prefix, supla_mqtt_message_provider *before,
                supla_mqtt_message_provider *after);
};

#endif /*MQTT_CHANNEL_REMOVED_TOPICS_PROVIDER_H_*/
