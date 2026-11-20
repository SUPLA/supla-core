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

#include <cstdio>
#include <list>
#include <memory>
#include <string>

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
  if (context->get_scope() == MQTTDS_SCOPE_FULL ||
      context->get_scope() == MQTTDS_SCOPE_USER ||
      context->get_scope() == MQTTDS_SCOPE_DEVICE) {
    lock();
    for (auto it = diagnostic_rows.begin(); it != diagnostic_rows.end();) {
      const auto &row = it->second;
      if (context->get_scope() == MQTTDS_SCOPE_FULL ||
          (context->get_scope() == MQTTDS_SCOPE_USER &&
           row.user_id == context->get_user_id()) ||
          (context->get_scope() == MQTTDS_SCOPE_DEVICE &&
           row.device_id == context->get_device_id())) {
        it = diagnostic_rows.erase(it);
      } else {
        ++it;
      }
    }
    unlock();
    if (context->get_scope() == MQTTDS_SCOPE_FULL) {
      diagnostic_publications.clear();
      lock();
      channel_owners.clear();
      subdevice_names.clear();
      retired_diagnostics.clear();
      unlock();
    }
  }

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
      remember_diagnostic_metadata(
          *static_cast<_mqtt_db_data_row_channel_t *>(data_row));
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
        ((now.tv_sec * 1000000ULL + now.tv_usec) -
         (actions.time.tv_sec * 1000000ULL + actions.time.tv_usec)) /
        1000ULL;

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
  for (int attempts = 0; attempts < 256; ++attempts) {
    if (!fetch_diagnostics(topic_name, message, message_size) &&
        !supla_mqtt_client_db_datasource::fetch(
            topic_name, message, message_size, retain)) {
      return false;
    }
    if (!repeated_diagnostic(*topic_name, *message, *message_size)) {
      return true;
    }
    free(*topic_name);
    free(*message);
    *topic_name = nullptr;
    *message = nullptr;
    *message_size = 0;
  }
  return false;
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
          users_enabled.erase(it);
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


void supla_mqtt_publisher_datasource::remember_diagnostic_metadata(
    const _mqtt_db_data_row_channel_t &row) {
  lock();
  const auto previous = channel_owners.find(row.channel_id);
  if (previous != channel_owners.end() &&
      previous->second.retired) {
    unlock();
    return;
  }
  channel_owners[row.channel_id] = {row.device_id, row.sub_device_id};
  const unsigned int owners[] = {0, row.sub_device_id};
  for (const auto owner : owners) {
    const diagnostic_key key{row.device_id, owner};
    auto [it, inserted] = diagnostic_rows.emplace(key, row);
    if (!inserted && it->second.channel_id != row.channel_id) {
      continue;
    }
    it->second = row;
    retired_diagnostics.erase(key);
    const auto name = subdevice_names.find(key);
    if (name != subdevice_names.end()) {
      snprintf(it->second.sub_device_name,
               sizeof(it->second.sub_device_name), "%s", name->second.c_str());
    }
    // Supply availability after initial Discovery, even if telemetry is idle.
    diagnostic_queue.emplace(key, false);
  }
  unlock();
}

void supla_mqtt_publisher_datasource::on_diagnostics_changed(
    int device_id, unsigned int owner) {
  lock();
  const diagnostic_key key{device_id, owner};
  if (diagnostic_rows.count(key)) {
    diagnostic_queue.emplace(key, false);
  }
  unlock();
}

void supla_mqtt_publisher_datasource::on_subdevice_name_changed(
    int device_id, unsigned int owner, const char *name) {
  std::string normalized = name ? name : "";
  const auto first = normalized.find_first_not_of(' ');
  normalized = first == std::string::npos ? ""
      : normalized.substr(first, normalized.find_last_not_of(' ') - first + 1);
  lock();
  const diagnostic_key key{device_id, owner};
  const auto found = diagnostic_rows.find(key);
  if (found == diagnostic_rows.end()) {
    unlock();
    return;
  }
  subdevice_names[key] = normalized;
  if (normalized != found->second.sub_device_name) {
    snprintf(found->second.sub_device_name,
             sizeof(found->second.sub_device_name), "%s", normalized.c_str());
    diagnostic_queue[key] = true;
  }
  unlock();
}

void supla_mqtt_publisher_datasource::on_channel_deleted(
    int device_id, int channel_id) {
  lock();
  const auto known = channel_owners.find(channel_id);
  if (known == channel_owners.end() || known->second.retired ||
      known->second.device_id != device_id) {
    unlock();
    return;
  }
  const unsigned int owner = known->second.owner;
  // Cloud removes all channels of the same subdevice in one transaction.
  if (owner) {
    const diagnostic_key key{device_id, owner};
    const auto metadata = diagnostic_rows.find(key);
    if (metadata != diagnostic_rows.end()) {
      diagnostic_withdrawals.emplace_back(metadata->second, owner);
      diagnostic_rows.erase(metadata);
    }
    diagnostic_queue.erase(key);
    subdevice_names.erase(key);
  }
  for (auto it = channel_owners.begin(); it != channel_owners.end();) {
    if (it->first == channel_id ||
        (owner && it->second.device_id == device_id &&
         it->second.owner == owner)) {
      it->second.retired = true;
    }
    ++it;
  }
  unlock();
}

void supla_mqtt_publisher_datasource::on_device_deleted(int device_id) {
  lock();
  for (auto it = diagnostic_rows.begin(); it != diagnostic_rows.end();) {
    if (it->first.first == device_id) {
      diagnostic_withdrawals.emplace_back(it->second, it->first.second);
      diagnostic_queue.erase(it->first);
      subdevice_names.erase(it->first);
      it = diagnostic_rows.erase(it);
    } else {
      ++it;
    }
  }
  for (auto it = channel_owners.begin(); it != channel_owners.end();) {
    if (it->second.device_id == device_id) {
      it->second.retired = true;
    }
    ++it;
  }
  unlock();
}

bool supla_mqtt_publisher_datasource::fetch_diagnostics(
    char **topic, void **message, size_t *size) {
  while (true) {
    if (diagnostic_provider) {
      if (diagnostic_provider->fetch(
              get_settings()->getPrefix(), topic,
              diagnostic_withdrawal ? nullptr : message,
              diagnostic_withdrawal ? nullptr : size)) {
        return true;
      }
      diagnostic_provider.reset();
    }
    lock();
    if (!diagnostic_withdrawals.empty()) {
      diagnostic_row = diagnostic_withdrawals.front().first;
      const auto owner = diagnostic_withdrawals.front().second;
      diagnostic_withdrawals.pop_front();
      unlock();
      retired_diagnostics.emplace(diagnostic_row.device_id, owner);
      diagnostic_provider =
          std::make_unique<supla_mqtt_channel_message_provider>();
      diagnostic_provider->set_diagnostics_only(owner);
      diagnostic_provider->set_data_row(&diagnostic_row);
      diagnostic_withdrawal = true;
      continue;
    }
    if (diagnostic_queue.empty()) {
      unlock();
      return false;
    }
    const auto request = *diagnostic_queue.begin();
    diagnostic_queue.erase(diagnostic_queue.begin());
    const auto found = diagnostic_rows.find(request.first);
    if (found == diagnostic_rows.end()) {
      unlock();
      continue;
    }
    diagnostic_row = found->second;
    unlock();
    if (!is_user_enabled(diagnostic_row.user_id)) {
      if (is_context_open()) {
        lock();
        diagnostic_queue.emplace(request);
        unlock();
        return false;
      }
      continue;
    }
    diagnostic_provider =
        std::make_unique<supla_mqtt_channel_message_provider>();
    if (!request.second) {
      diagnostic_provider->set_diagnostics_only(request.first.second);
    }
    diagnostic_provider->set_data_row(&diagnostic_row);
    diagnostic_withdrawal = false;
  }
}

bool supla_mqtt_publisher_datasource::repeated_diagnostic(
    const char *topic, const void *message, size_t size) {
  if (!topic) {
    return false;
  }
  const std::string key(topic);
  const bool diagnostic =
      key.find("/battery_") != std::string::npos ||
      key.find("/network_") != std::string::npos ||
      key.find("/subdevices/") != std::string::npos ||
      key.find("/state/battery_level") != std::string::npos ||
      key.find("/state/battery_cover_open") != std::string::npos ||
      key.find("/state/percentage") != std::string::npos ||
      key.find("/state/ip_address") != std::string::npos ||
      key.find("/state/wifi_rssi") != std::string::npos ||
      key.find("/last_connected") != std::string::npos ||
      key.find("/gateway_") != std::string::npos;
  if (!diagnostic) {
    return false;
  }
  if (message && size) {
    int device_id = 0;
    unsigned int owner = 0;
    const char *marker = nullptr;
    if (strncmp(topic, "homeassistant/", 14) == 0) {
      if ((marker = strstr(topic, "/network_"))) {
        sscanf(marker, "/network_%d_%u_", &device_id, &owner);
      } else if ((marker = strstr(topic, "/battery_"))) {
        sscanf(marker, "/battery_%d_%u", &device_id, &owner);
      } else if ((marker = strstr(topic, "/gateway_"))) {
        sscanf(marker, "/gateway_%d_last_connected", &device_id);
      }
    } else if ((marker = strstr(topic, "/devices/"))) {
      sscanf(marker, "/devices/%d/subdevices/%u/", &device_id, &owner);
      int channel = 0;
      if (sscanf(marker, "/devices/%d/channels/%d/",
                 &device_id, &channel) == 2) {
        lock();
        const auto found = channel_owners.find(channel);
        const bool deleted = found != channel_owners.end() &&
                             found->second.retired;
        unlock();
        if (deleted) {
          return true;
        }
      }
    }
    if (retired_diagnostics.count({device_id, owner})) {
      return true;
    }
  }
  const std::string value(message ? static_cast<const char *>(message) : "",
                          message ? size : 0);
  const auto found = diagnostic_publications.find(key);
  if (found != diagnostic_publications.end() && found->second == value) {
    return true;
  }
  diagnostic_publications[key] = value;
  return false;
}
