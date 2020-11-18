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

#include <mqtt_publisher_datasource.h>
#include <string.h>

supla_mqtt_publisher_datasource::supla_mqtt_publisher_datasource(void)
    : supla_mqtt_client_db_datasource() {
  memset(&data_row, 0, sizeof(_db_mqtt_data_row_t));
}

supla_mqtt_publisher_datasource::~supla_mqtt_publisher_datasource(void) {}

void *supla_mqtt_publisher_datasource::cursor_init(
    const _mqtt_ds_context_t *context) {
  if (db_connect()) {
    void *query = get_db()->mqtt_open_query(
        context->user_id, context->device_id, context->channel_id, &data_row);
    if (!query) {
      db_disconnect();
    }

    return query;
  }

  return NULL;
}

bool supla_mqtt_publisher_datasource::_fetch(const _mqtt_ds_context_t *context,
                                             void *cursor, char **topic_name,
                                             void **message,
                                             size_t *message_size, bool *eof) {
  return false;
}

void supla_mqtt_publisher_datasource::cursor_release(
    const _mqtt_ds_context_t *context, void *cursor) {
  if (get_db()) {
    get_db()->mqtt_close_query(cursor);
    db_disconnect();
  }
}
