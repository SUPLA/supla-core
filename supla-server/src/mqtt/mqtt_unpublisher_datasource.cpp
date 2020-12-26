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

#include <mqtt_unpublisher_datasource.h>
#include <string.h>

#define SUBSCRIPTION_TIME_SEC 20

supla_mqtt_unpublisher_datasource::supla_mqtt_unpublisher_datasource(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_db_datasource(settings) {
  user_topic_provider = new supla_mqtt_unpublisher_user_topic_provider();
  memset(&row, 0, sizeof(_mqtt_db_data_row_user_t));
}

supla_mqtt_unpublisher_datasource::~supla_mqtt_unpublisher_datasource(void) {
  delete user_topic_provider;
}

void supla_mqtt_unpublisher_datasource::thread_init(void) {
  supla_mqtt_client_db_datasource::thread_init();

  lock();
  users.clear();
  unlock();

  if (db_connect()) {
    void *query = get_db()->open_mqttenabledquery();
    if (query) {
      int user_id = 0;

      lock();
      while ((user_id = get_db()->mqttenabledquery_fetch_row(query))) {
        _unpub_user_item_t user;
        memset(&user, 0, sizeof(_unpub_user_item_t));
        user.user_id = user_id;
        user.mqtt_enabled = true;
        users.push_back(user);
      }
      unlock();

      get_db()->close_mqttenabledquery(query);
    }
    db_disconnect();
  }
}

bool supla_mqtt_unpublisher_datasource::is_user_enabled(int user_id) {
  bool result = false;
  lock();
  for (std::list<_unpub_user_item_t>::iterator it = users.begin();
       it != users.end(); ++it) {
    if (it->user_id == user_id) {
      result = it->mqtt_enabled;
      break;
    }
  }
  unlock();
  return result;
}

bool supla_mqtt_unpublisher_datasource::is_context_allowed(
    supla_mqtt_ds_context *context) {
  return context->get_scope() == MQTTDS_SCOPE_DEVICE &&
         is_user_enabled(context->get_user_id());
}

bool supla_mqtt_unpublisher_datasource::context_open(
    supla_mqtt_ds_context *context) {
  return false;
}

void supla_mqtt_unpublisher_datasource::on_userdata_changed(int user_id) {
  supla_mqtt_db *mqtt_db = new supla_mqtt_db();
  if (mqtt_db == NULL) {
    return;
  }

  if (mqtt_db->connect()) {
    bool mqtt_enabled = mqtt_db->mqtt_enabled(user_id);

    lock();
    bool exists = false;
    for (std::list<_unpub_user_item_t>::iterator it = users.begin();
         it != users.end(); ++it) {
      if (it->user_id == user_id) {
        exists = true;
        if (it->mqtt_enabled != mqtt_enabled) {
          it->mqtt_enabled = mqtt_enabled;

          if (mqtt_enabled) {
            it->needs_subscribe = false;
            it->subscribe_timeto.tv_sec = 1;
          } else {
            it->needs_subscribe = true;
            it->subscribe_timeto.tv_sec = 0;
            it->subscribe_timeto.tv_usec = 0;
          }
        }
        break;
      }
    }

    if (!exists && mqtt_enabled) {
      _unpub_user_item_t user;
      memset(&user, 0, sizeof(_unpub_user_item_t));
      user.user_id = user_id;
      user.mqtt_enabled = mqtt_enabled;
      users.push_back(user);
    }
    unlock();

    db_disconnect();
  }

  delete mqtt_db;
  mqtt_db = NULL;

  supla_mqtt_client_db_datasource::on_userdata_changed(user_id);
}

void supla_mqtt_unpublisher_datasource::before_channel_function_change(
    int UserID, int ChannelID) {}

void supla_mqtt_unpublisher_datasource::before_device_delete(int UserID,
                                                             int DeviceID) {}

bool supla_mqtt_unpublisher_datasource::_fetch(supla_mqtt_ds_context *context,
                                               char **topic_name,
                                               void **message,
                                               size_t *message_size) {
  bool result = false;

  lock();
  for (std::list<_unpub_user_item_t>::iterator it = users.begin();
       it != users.end(); ++it) {
  }

  unlock();

  return result;
}

void supla_mqtt_unpublisher_datasource::context_close(
    supla_mqtt_ds_context *context) {}

bool supla_mqtt_unpublisher_datasource::fetch_subscription(char **topic_name,
                                                           bool *unsubscribe) {
  if (user_topic_provider->fetch(get_settings()->getPrefix(), topic_name,
                                 unsubscribe)) {
    return true;
  }

  _unpub_user_item_t user;
  memset(&user, 0, sizeof(_unpub_user_item_t));

  struct timeval now;
  gettimeofday(&now, NULL);

  lock();
  for (std::list<_unpub_user_item_t>::iterator it = users.begin();
       it != users.end(); ++it) {
    if ((it->needs_subscribe && !it->subscribe_timeto.tv_sec) ||
        (it->subscribe_timeto.tv_sec &&
         it->subscribe_timeto.tv_sec <= now.tv_sec)) {
      user = *it;
      if (it->needs_subscribe) {
        it->subscribe_timeto = now;
        it->subscribe_timeto.tv_sec += SUBSCRIPTION_TIME_SEC;
        it->needs_subscribe = false;
      } else {
        users.erase(it);
      }
      break;
    }
  }
  unlock();

  if (user.user_id && db_connect()) {
    void *query = get_db()->open_userquery(user.user_id, false, &row);
    if (query) {
      if (get_db()->userquery_fetch_row(query)) {
        user_topic_provider->set_data_row(&row, !user.needs_subscribe);
      }
      get_db()->close_userquery(query);
    }

    db_disconnect();
  }

  return user_topic_provider->fetch(get_settings()->getPrefix(), topic_name,
                                    unsubscribe);
}
