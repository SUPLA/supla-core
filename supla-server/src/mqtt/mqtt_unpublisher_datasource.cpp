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

supla_mqtt_unpublisher_datasource::supla_mqtt_unpublisher_datasource(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_db_datasource(settings) {}

supla_mqtt_unpublisher_datasource::~supla_mqtt_unpublisher_datasource(void) {}

void supla_mqtt_unpublisher_datasource::thread_init(void) {
  supla_mqtt_client_db_datasource::thread_init();
}

bool supla_mqtt_unpublisher_datasource::is_context_allowed(
    supla_mqtt_ds_context *context) {
  return context->get_scope() == MQTTDS_SCOPE_DEVICE;
}

bool supla_mqtt_unpublisher_datasource::context_open(
    supla_mqtt_ds_context *context) {
  return false;
}

/*

 void supla_mqtt_unpublisher::on_userdata_changed(int user_id) {
  bool mqtt_enabled = false;
  supla_mqtt_db *mqtt_db = new supla_mqtt_db();

  if (mqtt_db) {
    if (mqtt_db->connect()) {
      mqtt_enabled = mqtt_db->mqtt_enabled(user_id);
    } else {
      user_id = 0;
    }
    delete mqtt_db;
  } else {
    user_id = 0;
  }

  if (user_id == 0) {
    return;
  }

  lck_lock(lck);
  bool add = true;
  for (std::list<_unpub_user_item_t>::iterator it = users.begin();
       it != users.end(); ++it) {
    if (it->user_id == user_id) {
      if (it->enabled != mqtt_enabled) {
        it->mqtt_enabled = mqtt_enabled;
        it->time.tv_sec = 0;
        it->time.tv_usec = 0;
      }
      add = false;
      break;
    }
  }

  if (add) {
    _unpub_user_item_t user;
    memset(&user, 0, sizeof(_unpub_user_item_t));
    user.user_id = user_id;
    user.mqtt_enabled = mqtt_enabled;
    users.pop_back(user);
  }

  lck_unlock(lck);

  supla_mqtt_client::on_userdata_changed(user_id);
}

int supla_mqtt_unpublisher::user_id_pop(bool *subscribe) {
  _unpub_user_item_t user;
  memset(&user, 0, sizeof(_unpub_user_item_t));

  int user_id = 0;
  struct timeval now;
  gettimeofday(&now, NULL);

  lck_lock(lck);
  for (std::list<_unpub_user_item_t>::iterator it = users.begin();
       it != users.end(); ++it) {
    if (it->mqtt_enabled) {
      *subscribe = false;
      user_id = it->user_id;
      users.erase(it);
      break;
    } else {
      if (it->time.tv_sec) {
        if (now.tv_sec - it->time.tv_sec >= SUBSCRIPTION_TIME_SEC) {
          *subscribe = false;
          user_id = it->user_id;
          users.erase(it);
          break;
        }
      } else {
        *subscribe = true;
        user_id = it->user_id;
        it->time = now;
        break;
      }
    }
  }
  lck_unlock(lck);

  return user_id;
}
 */

bool supla_mqtt_unpublisher_datasource::_fetch(supla_mqtt_ds_context *context,
                                               char **topic_name,
                                               void **message,
                                               size_t *message_size) {
  return false;
}

void supla_mqtt_unpublisher_datasource::context_close(
    supla_mqtt_ds_context *context) {}

void supla_mqtt_unpublisher_datasource::on_userdata_changed(int user_id) {
  supla_mqtt_client_datasource::on_userdata_changed(user_id);
}

bool supla_mqtt_unpublisher_datasource::fetch_subscription(char **topic_name,
                                                           bool *unsubscribe) {}
