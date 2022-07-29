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

using std::list;

#define MPD_DATATYPE_USER 1
#define MPD_DATATYPE_DEVICE 2
#define MPD_DATATYPE_CHANNEL 3

#define ACTION_TIMEOUT_MSEC 3000

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
  this->action_message_provider = NULL;
}

supla_mqtt_publisher_datasource::~supla_mqtt_publisher_datasource(void) {
  if (user_message_provider) {
    delete user_message_provider;
  }

  if (device_message_provider) {
    delete device_message_provider;
  }

  if (channelandstate_message_provider) {
    delete channelandstate_message_provider;
  }

  if (state_message_provider) {
    delete state_message_provider;
  }

  if (action_message_provider) {
    delete action_message_provider;
  }
}

bool supla_mqtt_publisher_datasource::is_context_allowed(
    supla_mqtt_ds_context *context) {
  switch (context->get_scope()) {
    case MQTTDS_SCOPE_FULL:
    case MQTTDS_SCOPE_USER:
      return true;
    case MQTTDS_SCOPE_DEVICE:
    case MQTTDS_SCOPE_CHANNEL_STATE:
      return is_user_enabled(context->get_user_id());
    default:
      return false;
  }
}

bool supla_mqtt_publisher_datasource::is_user_enabled(int user_id) {
  bool result = false;
  lock();
  for (auto it = users_enabled.begin(); it != users_enabled.end(); ++it) {
    if (*it == user_id) {
      result = true;
      break;
    }
  }
  unlock();
  return result;
}

bool supla_mqtt_publisher_datasource::context_open(
    supla_mqtt_ds_context *context) {
  fetch_users = false;
  fetch_devices = false;
  fetch_channels = false;
  fetch_states = false;

  switch (context->get_scope()) {
    case MQTTDS_SCOPE_FULL:
    case MQTTDS_SCOPE_USER:
      fetch_users = true;
      fetch_devices = true;
      fetch_channels = true;
      users_enabled_tmp.clear();
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
      return new _mqtt_db_data_row_user_t();
    case MPD_DATATYPE_DEVICE:
      return new _mqtt_db_data_row_device_t();
    case MPD_DATATYPE_CHANNEL:
      return new _mqtt_db_data_row_channel_t();
      break;
  }

  return NULL;
}

void *supla_mqtt_publisher_datasource::open_query(
    int datatype, supla_mqtt_ds_context *context, void *data_row) {
  switch (datatype) {
    case MPD_DATATYPE_USER:
      return get_db()->open_userquery(
          context->get_user_id(), true,
          static_cast<_mqtt_db_data_row_user_t *>(data_row));
    case MPD_DATATYPE_DEVICE:
      return get_db()->open_devicequery(
          context->get_user_id(), context->get_device_id(),
          static_cast<_mqtt_db_data_row_device_t *>(data_row));
    case MPD_DATATYPE_CHANNEL:
      return get_db()->open_channelquery(
          context->get_user_id(), context->get_device_id(),
          context->get_channel_id(),
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
    supla_mqtt_ds_context *context, int datatype,
    supla_mqtt_message_provider *provider, void *data_row) {
  switch (datatype) {
    case MPD_DATATYPE_USER: {
      _mqtt_db_data_row_user_t *row_user =
          static_cast<_mqtt_db_data_row_user_t *>(data_row);
      if (row_user) {
        if (context->get_scope() == MQTTDS_SCOPE_FULL) {
          context->set_user_id(row_user->user_id);
        }
        static_cast<supla_mqtt_user_message_provider *>(provider)->set_data_row(
            row_user);
      }
    } break;
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
    supla_mqtt_message_provider **provider, supla_mqtt_ds_context *context,
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
      set_provider_data_row(context, datatype, *provider, *data_row);
    }

    if (*provider) {
      if ((*provider)->fetch(get_settings()->getPrefix(), topic_name, message,
                             message_size)) {
        return true;
      } else {
        if (!db_fetch_row(datatype, *query, *data_row)) {
          return false;
        }
        set_provider_data_row(context, datatype, *provider, *data_row);
      }
      return (*provider)->fetch(get_settings()->getPrefix(), topic_name,
                                message, message_size);
    }
  }

  return false;
}

bool supla_mqtt_publisher_datasource::fetch_actions(char **topic_name,
                                                    void **message,
                                                    size_t *message_size) {
  struct timeval now;
  gettimeofday(&now, NULL);
  unsigned long long time_diff = 0;

  lock();
  int queue_size = triggered_actions.size();
  _mqtt_ds_triggered_actions_t actions = {};
  bool action_exists = false;

  if (queue_size > 0) {
    actions = triggered_actions.front();
    unsigned long long time_diff =
        ((now.tv_sec * 1000000 + now.tv_usec) -
         (actions.time.tv_sec * 1000000 + actions.time.tv_usec)) /
        1000;

    if (time_diff >= ACTION_TIMEOUT_MSEC) {
      triggered_actions.pop_front();
    } else {
      unsigned int bit = 1;
      time_diff = 0;

      for (long unsigned int n = 0; n < sizeof(actions.actions) * 8; n++) {
        if (actions.actions & bit) {
          actions.actions = actions.actions & (~bit);
          if (actions.actions) {
            triggered_actions.front().actions = actions.actions;
          } else {
            triggered_actions.pop_front();
          }
          actions.actions = bit;
          action_exists = true;
          break;
        }
        bit <<= 1;
      }
    }
  }

  unlock();

  if (!queue_size) {
    if (action_message_provider) {
      delete action_message_provider;
      action_message_provider = NULL;
    }
  } else {
    if (!action_message_provider) {
      action_message_provider = new supla_mqtt_action_message_provider();
    }
  }

  if (time_diff) {
    supla_log(
        LOG_WARNING,
        "MQTT - Action trigger publication timeout: %llu msec. ChannelID: "
        "%i, Queue size %i",
        time_diff, actions.channel_id, queue_size);
    return false;
  }

  if (!action_exists || !action_message_provider) {
    return false;
  }

  action_message_provider->reset_index();
  action_message_provider->set_action(actions.user_id, actions.device_id,
                                      actions.channel_id, actions.actions);
  return action_message_provider->fetch(get_settings()->getPrefix(), topic_name,
                                        message, message_size);
}

bool supla_mqtt_publisher_datasource::fetch_user(supla_mqtt_ds_context *context,
                                                 char **topic_name,
                                                 void **message,
                                                 size_t *message_size) {
  return fetch(MPD_DATATYPE_USER, &user_query, &userdata_row,
               &user_message_provider, context, topic_name, message,
               message_size);
}

bool supla_mqtt_publisher_datasource::fetch_device(
    supla_mqtt_ds_context *context, char **topic_name, void **message,
    size_t *message_size) {
  return fetch(MPD_DATATYPE_DEVICE, &device_query, &devicedata_row,
               &device_message_provider, context, topic_name, message,
               message_size);
}

bool supla_mqtt_publisher_datasource::fetch_channel(
    supla_mqtt_ds_context *context, char **topic_name, void **message,
    size_t *message_size) {
  return fetch(MPD_DATATYPE_CHANNEL, &channel_query, &channeldata_row,
               &channelandstate_message_provider, context, topic_name, message,
               message_size);
}

bool supla_mqtt_publisher_datasource::fetch_state(
    supla_mqtt_ds_context *context, char **topic_name, void **message,
    size_t *message_size) {
  if (state_message_provider == NULL) {
    state_message_provider = new supla_mqtt_state_message_provider();
    state_message_provider->set_ids(context->get_user_id(),
                                    context->get_device_id(),
                                    context->get_channel_id());
    state_message_provider->set_user_suid();
  }

  if (state_message_provider) {
    return state_message_provider->fetch(get_settings()->getPrefix(),
                                         topic_name, message, message_size);
  }

  return false;
}

bool supla_mqtt_publisher_datasource::_fetch(supla_mqtt_ds_context *context,
                                             char **topic_name, void **message,
                                             size_t *message_size,
                                             bool *retain) {
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

  if (result) {
    if (context->get_user_id()) {
      if (context->get_scope() == MQTTDS_SCOPE_FULL) {
        add_user_to_list(context->get_user_id(), &users_enabled_tmp);
      } else if (context->get_scope() == MQTTDS_SCOPE_USER &&
                 users_enabled_tmp.size() == 0) {
        users_enabled_tmp.push_back(context->get_user_id());
      }
    }
  }

  return result;
}

bool supla_mqtt_publisher_datasource::fetch(char **topic_name, void **message,
                                            size_t *message_size,
                                            bool *retain) {
  if (fetch_actions(topic_name, message, message_size)) {
    if (retain) {
      *retain = false;
    }

    return true;
  }

  *retain = true;

  return supla_mqtt_client_db_datasource::fetch(topic_name, message,
                                                message_size, retain);
}

void supla_mqtt_publisher_datasource::close_userquery(void) {
  if (user_query) {
    if (get_db()) {
      get_db()->close_userquery(user_query);
    }
    user_query = NULL;
  }

  if (userdata_row) {
    delete static_cast<_mqtt_db_data_row_user_t *>(userdata_row);
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
    delete static_cast<_mqtt_db_data_row_device_t *>(devicedata_row);
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
    delete static_cast<_mqtt_db_data_row_channel_t *>(channeldata_row);
    channeldata_row = NULL;
  }

  if (channelandstate_message_provider) {
    delete static_cast<supla_mqtt_channelandstate_message_provider *>(
        channelandstate_message_provider);
    channelandstate_message_provider = NULL;
  }
}

void supla_mqtt_publisher_datasource::add_user_to_list(int user_id,
                                                       list<int> *ulist) {
  bool exists = false;
  for (auto it = ulist->begin(); it != ulist->end(); ++it) {
    if (*it == user_id) {
      exists = true;
      break;
    }
  }

  if (!exists) {
    ulist->push_back(user_id);
  }
}

void supla_mqtt_publisher_datasource::context_close(
    supla_mqtt_ds_context *context) {
  lock();
  if (context->get_scope() == MQTTDS_SCOPE_FULL) {
    users_enabled = users_enabled_tmp;
  } else if (context->get_scope() == MQTTDS_SCOPE_USER) {
    if (users_enabled_tmp.size()) {
      add_user_to_list(users_enabled_tmp.front(), &users_enabled);
    } else if (context->get_user_id()) {
      for (auto it = users_enabled.begin(); it != users_enabled.end(); ++it) {
        if (*it == context->get_user_id()) {
          it = users_enabled.erase(it);
          break;
        }
      }
    }
  }

  users_enabled_tmp.clear();
  unlock();

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

void supla_mqtt_publisher_datasource::on_actions_triggered(
    int user_id, int device_id, int channel_id, unsigned int actions) {
  if (!user_id || !device_id || !channel_id) {
    return;
  }

  actions &= SUPLA_ACTION_CAP_TURN_ON | SUPLA_ACTION_CAP_TURN_OFF |
             SUPLA_ACTION_CAP_TOGGLE_x1 | SUPLA_ACTION_CAP_TOGGLE_x2 |
             SUPLA_ACTION_CAP_TOGGLE_x3 | SUPLA_ACTION_CAP_TOGGLE_x4 |
             SUPLA_ACTION_CAP_TOGGLE_x5 | SUPLA_ACTION_CAP_HOLD |
             SUPLA_ACTION_CAP_SHORT_PRESS_x1 | SUPLA_ACTION_CAP_SHORT_PRESS_x2 |
             SUPLA_ACTION_CAP_SHORT_PRESS_x3 | SUPLA_ACTION_CAP_SHORT_PRESS_x4 |
             SUPLA_ACTION_CAP_SHORT_PRESS_x5;

  if (!actions) {
    return;
  }

  struct timeval now;
  gettimeofday(&now, NULL);

  lock();
  bool exists = false;
  for (auto it = triggered_actions.begin(); it != triggered_actions.end();
       ++it) {
    if (it->user_id == user_id && it->device_id == device_id &&
        it->channel_id == channel_id) {
      exists = true;
      it->actions |= actions;
      it->time = now;
      break;
    }
  }

  if (!exists && is_user_enabled(user_id)) {
    _mqtt_ds_triggered_actions_t _actions = {};
    _actions.user_id = user_id;
    _actions.device_id = device_id;
    _actions.channel_id = channel_id;
    _actions.time = now;
    _actions.actions = actions;
    triggered_actions.push_back(_actions);
  }

  unlock();
}
