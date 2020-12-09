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
#include "log.h"
#include "mqtt_channelandstate_message_provider.h"
#include "mqtt_device_message_provider.h"
#include "mqtt_user_message_provider.h"

#define MPD_DATATYPE_USER 1
#define MPD_DATATYPE_DEVICE 2
#define MPD_DATATYPE_CHANNEL 3

supla_mqtt_publisher_datasource::supla_mqtt_publisher_datasource(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_db_datasource(settings) {
  this->fetch_users = false;
  this->fetch_devices = false;
  this->fetch_channels = false;
  this->fetch_states = false;
  this->user_query = NULL;
  this->device_query = NULL;
  this->channel_query = NULL;
  this->userdata_row = NULL;
  this->devicedata_row = NULL;
  this->channeldata_row = NULL;
  this->user_message_provider = NULL;
  this->device_message_provider = NULL;
  this->channelandstate_message_provider = NULL;
  this->state_message_provider = NULL;
}

supla_mqtt_publisher_datasource::~supla_mqtt_publisher_datasource(void) {}

bool supla_mqtt_publisher_datasource::context_open(
    const _mqtt_ds_context_t *context) {
  fetch_users = false;
  fetch_devices = false;
  fetch_channels = false;
  fetch_states = false;

  switch (context->scope) {
    case MQTTDS_SCOPE_FULL:
    case MQTTDS_SCOPE_USER:
      fetch_users = true;
      fetch_devices = true;
      fetch_channels = true;
      break;
    case MQTTDS_SCOPE_DEVICE:
      fetch_devices = true;
      fetch_channels = true;
      break;
    case MQTTDS_SCOPE_CHANNEL_STATE:
      fetch_states = true;
      break;
    default:
      break;
  }

  return true;
}

void *supla_mqtt_publisher_datasource::datarow_malloc(int datatype) {
  switch (datatype) {
    case MPD_DATATYPE_USER:
      return malloc(sizeof(_mqtt_db_data_row_user_t));
    case MPD_DATATYPE_DEVICE:
      return malloc(sizeof(_mqtt_db_data_row_device_t));
    case MPD_DATATYPE_CHANNEL:
      return malloc(sizeof(_mqtt_db_data_row_channel_t));
  }

  return NULL;
}

void *supla_mqtt_publisher_datasource::open_query(
    int datatype, const _mqtt_ds_context_t *context, void *data_row) {
  switch (datatype) {
    case MPD_DATATYPE_USER:
      return get_db()->open_userquery(
          context->user_id, static_cast<_mqtt_db_data_row_user_t *>(data_row));
    case MPD_DATATYPE_DEVICE:
      return get_db()->open_devicequery(
          context->user_id, context->device_id,
          static_cast<_mqtt_db_data_row_device_t *>(data_row));
    case MPD_DATATYPE_CHANNEL:
      return get_db()->open_channelquery(
          context->user_id, context->device_id, context->channel_id,
          static_cast<_mqtt_db_data_row_channel_t *>(data_row));
  }

  return NULL;
}

bool supla_mqtt_publisher_datasource::db_fetch_row(int datatype, void *query,
                                                   void *data_row) {
  switch (datatype) {
    case MPD_DATATYPE_USER:
      return get_db()->userquery_fetch_row(query);
    case MPD_DATATYPE_DEVICE:
      return get_db()->devicequery_fetch_row(query);
    case MPD_DATATYPE_CHANNEL:
      return get_db()->channelquery_fetch_row(query);
  }

  return false;
}

supla_mqtt_message_provider *supla_mqtt_publisher_datasource::new_provider(
    int datatype, void *data_row) {
  switch (datatype) {
    case MPD_DATATYPE_USER:
      return new supla_mqtt_user_message_provider();
    case MPD_DATATYPE_DEVICE:
      return new supla_mqtt_device_message_provider();
    case MPD_DATATYPE_CHANNEL:
      return new supla_mqtt_channelandstate_message_provider();
  }

  return NULL;
}

void supla_mqtt_publisher_datasource::set_provider_data_row(
    int datatype, supla_mqtt_message_provider *provider, void *data_row) {
  switch (datatype) {
    case MPD_DATATYPE_USER:
      static_cast<supla_mqtt_user_message_provider *>(provider)->set_data_row(
          static_cast<_mqtt_db_data_row_user_t *>(data_row));
      break;
    case MPD_DATATYPE_DEVICE:
      static_cast<supla_mqtt_device_message_provider *>(provider)->set_data_row(
          static_cast<_mqtt_db_data_row_device_t *>(data_row));
      break;
    case MPD_DATATYPE_CHANNEL:
      static_cast<supla_mqtt_channelandstate_message_provider *>(provider)
          ->set_data_row(static_cast<_mqtt_db_data_row_channel_t *>(data_row));
      break;
  }
}

bool supla_mqtt_publisher_datasource::fetch(
    int datatype, void **query, void **data_row,
    supla_mqtt_message_provider **provider, const _mqtt_ds_context_t *context,
    char **topic_name, void **message, size_t *message_size) {
  if (*query == NULL) {
    if (db_connect() && get_db()) {
      if (*data_row == NULL) {
        *data_row = datarow_malloc(datatype);
      }
      if (*data_row) {
        *query = open_query(datatype, context, *data_row);
        if (*query && !db_fetch_row(datatype, *query, *data_row)) {
          return false;
        }
      }
    }
  }

  if (*query) {
    if (*provider == NULL) {
      *provider = new_provider(datatype, *data_row);
      set_provider_data_row(datatype, *provider, *data_row);
    }

    if (*provider) {
      if ((*provider)->fetch(get_settings()->getPrefix(), topic_name, message,
                             message_size)) {
        return true;
      } else {
        if (!db_fetch_row(datatype, *query, *data_row)) {
          return false;
        }
        set_provider_data_row(datatype, *provider, *data_row);
      }
      return (*provider)->fetch(get_settings()->getPrefix(), topic_name,
                                message, message_size);
    }
  }

  return false;
}

bool supla_mqtt_publisher_datasource::fetch_user(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  return fetch(MPD_DATATYPE_USER, &user_query, &userdata_row,
               &user_message_provider, context, topic_name, message,
               message_size);
}

bool supla_mqtt_publisher_datasource::fetch_device(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  return fetch(MPD_DATATYPE_DEVICE, &device_query, &devicedata_row,
               &device_message_provider, context, topic_name, message,
               message_size);
}

bool supla_mqtt_publisher_datasource::fetch_channel(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  return fetch(MPD_DATATYPE_CHANNEL, &channel_query, &channeldata_row,
               &channelandstate_message_provider, context, topic_name, message,
               message_size);
}

bool supla_mqtt_publisher_datasource::fetch_state(
    const _mqtt_ds_context_t *context, char **topic_name, void **message,
    size_t *message_size) {
  if (state_message_provider == NULL) {
    state_message_provider = new supla_mqtt_state_message_provider();
    state_message_provider->set_ids(context->user_id, context->device_id,
                                    context->channel_id);
    state_message_provider->set_user_email();
  }

  if (state_message_provider) {
    return state_message_provider->fetch(get_settings()->getPrefix(),
                                         topic_name, message, message_size);
  }

  return false;
}

bool supla_mqtt_publisher_datasource::_fetch(const _mqtt_ds_context_t *context,
                                             char **topic_name, void **message,
                                             size_t *message_size) {
  bool result = false;
  if (fetch_users) {
    result = fetch_user(context, topic_name, message, message_size);
    if (!result) {
      fetch_users = false;
      close_userquery();
    }
  }

  if (!result && fetch_devices) {
    result = fetch_device(context, topic_name, message, message_size);
    if (!result) {
      fetch_devices = false;
      close_devicequery();
    }
  }

  if (!result && fetch_channels) {
    result = fetch_channel(context, topic_name, message, message_size);
    if (!result) {
      fetch_channels = false;
      close_channelquery();
    }
  }

  if (!result && fetch_states) {
    result = fetch_state(context, topic_name, message, message_size);
  }

  return result;
}

void supla_mqtt_publisher_datasource::close_userquery(void) {
  if (user_query) {
    if (get_db()) {
      get_db()->close_userquery(user_query);
    }
    user_query = NULL;
  }

  if (userdata_row) {
    free(userdata_row);
    userdata_row = NULL;
  }

  if (user_message_provider) {
    delete static_cast<supla_mqtt_user_message_provider *>(
        user_message_provider);
    user_message_provider = NULL;
  }
}

void supla_mqtt_publisher_datasource::close_devicequery(void) {
  if (device_query) {
    if (get_db()) {
      get_db()->close_devicequery(device_query);
    }
    device_query = NULL;
  }

  if (devicedata_row) {
    free(devicedata_row);
    devicedata_row = NULL;
  }

  if (device_message_provider) {
    delete static_cast<supla_mqtt_device_message_provider *>(
        device_message_provider);
    device_message_provider = NULL;
  }
}

void supla_mqtt_publisher_datasource::close_channelquery(void) {
  if (channel_query) {
    if (get_db()) {
      get_db()->close_channelquery(channel_query);
    }
    channel_query = NULL;
  }

  if (channeldata_row) {
    free(channeldata_row);
    channeldata_row = NULL;
  }

  if (channelandstate_message_provider) {
    delete static_cast<supla_mqtt_channelandstate_message_provider *>(
        channelandstate_message_provider);
    channelandstate_message_provider = NULL;
  }
}

void supla_mqtt_publisher_datasource::context_close(
    const _mqtt_ds_context_t *context) {
  fetch_users = false;
  fetch_devices = false;
  fetch_channels = false;
  fetch_states = false;

  close_userquery();
  close_devicequery();
  close_channelquery();

  if (state_message_provider) {
    delete state_message_provider;
    state_message_provider = NULL;
  }

  db_disconnect();
}
