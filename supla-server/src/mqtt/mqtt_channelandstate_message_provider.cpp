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

#include <mqtt_channelandstate_message_provider.h>

supla_mqtt_channelandstate_message_provider::
    supla_mqtt_channelandstate_message_provider(void)
    : supla_mqtt_message_provider() {
  channel_message_provider = NULL;
  state_message_provider = NULL;
}

supla_mqtt_channelandstate_message_provider::
    ~supla_mqtt_channelandstate_message_provider(void) {
  delete channel_message_provider;
  channel_message_provider = NULL;
  delete state_message_provider;
  state_message_provider = NULL;
}

bool supla_mqtt_channelandstate_message_provider::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (channel_message_provider) {
    if (channel_message_provider->get_message_at_index(
            index, topic_prefix, topic_name, message, message_size)) {
      return true;
    } else {
      delete channel_message_provider;
      channel_message_provider = NULL;
    }
  }

  if (state_message_provider) {
    if (state_message_provider->get_message_at_index(
            index, topic_prefix, topic_name, message, message_size)) {
      return true;
    } else {
      delete state_message_provider;
      state_message_provider = NULL;
    }
  }
  return false;
}

void supla_mqtt_channelandstate_message_provider::init_providers_if_needed(
    void) {
  if (channel_message_provider == NULL) {
    channel_message_provider = new supla_mqtt_channel_message_provider();
  }

  if (state_message_provider == NULL) {
    state_message_provider = new supla_mqtt_state_message_provider();
  }
}

void supla_mqtt_channelandstate_message_provider::reset_index(void) {
  init_providers_if_needed();

  channel_message_provider->reset_index();
  state_message_provider->reset_index();
}

void supla_mqtt_channelandstate_message_provider::set_data_row(
    _mqtt_db_data_row_channel_t *row) {
  init_providers_if_needed();
  channel_message_provider->set_data_row(row);
  if (row) {
    state_message_provider->set_data(row->user_id, row->device_id,
                                     row->channel_id);
  } else {
    state_message_provider->set_data(0, 0, 0);
  }
}
