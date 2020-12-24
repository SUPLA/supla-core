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

supla_mqtt_unpublisher_datasource::supla_mqtt_unpublisher_datasource(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_db_datasource(settings) {
  user_topic_provider = NULL;
}

supla_mqtt_unpublisher_datasource::~supla_mqtt_unpublisher_datasource(void) {}

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
  if (db_connect()) {
    bool mqtt_enabled = get_db()->mqtt_enabled(user_id);

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
            it->subscribe_time.tv_sec = 1;
          } else {
            it->needs_subscribe = true;
            it->subscribe_time.tv_sec = 0;
            it->subscribe_time.tv_usec = 0;
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
  supla_mqtt_client_db_datasource::on_userdata_changed(user_id);
}

bool supla_mqtt_unpublisher_datasource::_fetch(supla_mqtt_ds_context *context,
                                               char **topic_name,
                                               void **message,
                                               size_t *message_size) {
  return false;
}

void supla_mqtt_unpublisher_datasource::context_close(
    supla_mqtt_ds_context *context) {}

bool supla_mqtt_unpublisher_datasource::fetch_subscription(char **topic_name,
                                                           bool *unsubscribe) {
  lock();
  for (std::list<_unpub_user_item_t>::iterator it = users.begin();
       it != users.end(); ++it) {
  }
  unlock();
}
