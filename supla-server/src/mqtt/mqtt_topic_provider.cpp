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

#include <mqtt_topic_provider.h>

supla_mqtt_topic_provider::supla_mqtt_topic_provider(void) {}

supla_mqtt_topic_provider::~supla_mqtt_topic_provider(void) {}

void supla_mqtt_topic_provider::datarow_changed(_db_mqtt_data_row_t *data_row) {

}

bool supla_mqtt_topic_provider::fetch(char **topic_name, void **message,
                                      size_t *message_size) {
  return false;
}
