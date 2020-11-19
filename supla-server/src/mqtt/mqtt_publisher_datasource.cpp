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
  this->fetch_users = false;
  this->fetch_devices = false;
  this->fetch_channels = false;
  this->fetch_values = false;
  this->user_query = NULL;
  this->device_query = NULL;
  this->channel_query = NULL;
  this->userdata_row = NULL;
}

supla_mqtt_publisher_datasource::~supla_mqtt_publisher_datasource(void) {}

bool supla_mqtt_publisher_datasource::context_open(
    const _mqtt_ds_context_t *context) {
  fetch_users = false;
  fetch_devices = false;
  fetch_channels = false;
  fetch_values = false;

  switch (context->scope) {
    case MQTTDS_SCOPE_FULL:
    case MQTTDS_SCOPE_USER:
      fetch_users = true;
      fetch_devices = true;
      fetch_channels = true;
      fetch_values = true;
      break;
    case MQTTDS_SCOPE_DEVICE:
      fetch_devices = true;
      fetch_channels = true;
      fetch_values = true;
      break;
    case MQTTDS_SCOPE_CHANNEL_VALUE:
      fetch_values = true;
      break;
  }

  return true;
}

bool supla_mqtt_publisher_datasource::fetch_user(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  if (user_query == NULL) {
    if (db_connect() && get_db()) {
      user_query =
          get_db()->mqtt_open_userquery(context->user_id, userdata_row);
    }
  }

  if (user_query) {

  }

  return false;
}

bool supla_mqtt_publisher_datasource::fetch_device(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  return false;
}

bool supla_mqtt_publisher_datasource::fetch_channel(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  return false;
}

bool supla_mqtt_publisher_datasource::fetch_value(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  return false;
}

bool supla_mqtt_publisher_datasource::_fetch(const _mqtt_ds_context_t *context,
                                             char **topic_name, void **message,
                                             size_t *message_size) {
  bool result = false;
  if (fetch_users) {
    result = fetch_user(context, topic_name, message, message_size);
  }

  if (!result) {
    fetch_users = false;
    close_userquery();

    if (fetch_devices) {
      result = fetch_device(context, topic_name, message, message_size);
    }
  }

  if (!result) {
    fetch_devices = false;
    close_devicequery();

    if (fetch_channels) {
      result = fetch_channel(context, topic_name, message, message_size);
    }
  }

  if (!result) {
    fetch_channels = false;
    close_channelquery();

    if (fetch_values) {
      result = fetch_value(context, topic_name, message, message_size);
    }
  }

  return result;
}

void supla_mqtt_publisher_datasource::close_userquery(void) {
  if (user_query) {
    if (get_db()) {
      get_db()->mqtt_close_userquery(user_query);
    }
    user_query = NULL;
  }

  if (userdata_row) {
    free(userdata_row);
    userdata_row = NULL;
  }
}

void supla_mqtt_publisher_datasource::close_devicequery(void) {
  if (device_query) {
    if (get_db()) {
      get_db()->mqtt_close_devicequery(user_query);
    }
    device_query = NULL;
  }

  if (devicedata_row) {
    free(devicedata_row);
    devicedata_row = NULL;
  }
}

void supla_mqtt_publisher_datasource::close_channelquery(void) {
  if (channel_query) {
    if (get_db()) {
      get_db()->mqtt_close_channelquery(user_query);
    }
    channel_query = NULL;
  }

  if (channeldata_row) {
    free(channeldata_row);
    channeldata_row = NULL;
  }
}

void supla_mqtt_publisher_datasource::context_close(
    const _mqtt_ds_context_t *context) {
  fetch_users = false;
  fetch_devices = false;
  fetch_channels = false;
  fetch_values = false;

  close_userquery();
  close_devicequery();
  close_channelquery();
}
