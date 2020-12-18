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

#include <mqtt_client_datasource.h>
#include <string.h>
#include "lck.h"

supla_mqtt_client_datasource::supla_mqtt_client_datasource(
    supla_mqtt_client_settings *settings) {
  lck = lck_init();
  _context_open = false;
  all_data_expected = false;
  this->settings = settings;
}

supla_mqtt_client_datasource::~supla_mqtt_client_datasource(void) {
  lck_free(lck);
}

supla_mqtt_client_settings *supla_mqtt_client_datasource::get_settings(void) {
  return settings;
}

void supla_mqtt_client_datasource::thread_init(void) {}

void supla_mqtt_client_datasource::thread_cleanup(void) { context_close(); }

bool supla_mqtt_client_datasource::is_mqtt_enabled(int user_id) {
  bool result = false;
  lck_lock(lck);
  for (std::list<int>::iterator it = users.begin(); it != users.end(); ++it) {
    if (*it == user_id) {
      result = true;
      break;
    }
  }
  lck_unlock(lck);
  return result;
}

void supla_mqtt_client_datasource::context_close(void) {
  if (is_context_open()) {
    context_close(&context);
    lck_lock(lck);
    _context_open = false;
    lck_unlock(lck);
  }
}

void supla_mqtt_client_datasource::context_open(void) {
  context_close();
  context_open(&context);
}

bool supla_mqtt_client_datasource::context_should_be_opened(void) {
  bool result = false;
  lck_lock(lck);

  if (!is_context_open()) {
    if (all_data_expected) {
      all_data_expected = false;
      context = supla_mqtt_ds_context(MQTTDS_SCOPE_FULL);
      result = true;
    } else if (user_queue.size()) {
      context = supla_mqtt_ds_context(MQTTDS_SCOPE_USER, user_queue.front());
      user_queue.pop_front();
      result = true;
    } else if (device_queue.size()) {
      _mqtt_ds_device_id_t id = device_queue.front();
      device_queue.pop_front();

      context =
          supla_mqtt_ds_context(MQTTDS_SCOPE_USER, id.user_id, id.device_id);
      result = true;
    } else if (channel_queue.size()) {
      context.scope = MQTTDS_SCOPE_CHANNEL_STATE;
      _mqtt_ds_channel_id_t id = channel_queue.front();
      channel_queue.pop_front();

      context = supla_mqtt_ds_context(MQTTDS_SCOPE_USER, id.user_id,
                                      id.device_id, id.channel_id);
      result = true;
    }
  }

  lck_unlock(lck);
  return result;
}

void supla_mqtt_client_datasource::add_user_to_list(int user_id,
                                                    std::list<int> *ulist) {
  bool exists = false;
  for (std::list<int>::iterator it = ulist->begin(); it != ulist->end(); ++it) {
    if (*it == user_id) {
      exists = true;
      break;
    }
  }

  if (!exists) {
    printf("%i\n", user_id);
    ulist->push_back(user_id);
  }
}

void supla_mqtt_client_datasource::update_user_list() {
  if (context.scope == MQTTDS_SCOPE_FULL) {
    users = users_tmp;
  } else if (context.scope == MQTTDS_SCOPE_USER) {
    if (users_tmp.size()) {
      add_user_to_list(users_tmp.front(), &users);
    } else if (context.user_id) {
      for (std::list<int>::iterator it = users.begin(); it != users.end();
           ++it) {
        if (*it == context.user_id) {
          it = users.erase(it);
          break;
        }
      }
    }
  }

  users_tmp.clear();
}

bool supla_mqtt_client_datasource::fetch(char **topic_name, void **message,
                                         size_t *message_size) {
  if (context_should_be_opened()) {
    bool __context_open = context_open(&context);
    lck_lock(lck);
    _context_open = __context_open;
    lck_unlock(lck);

    if (context.scope == MQTTDS_SCOPE_FULL ||
        context.scope == MQTTDS_SCOPE_USER) {
      users_tmp.clear();
    }
  }

  bool result = false;

  if (is_context_open()) {
    result = _fetch(&context, &user_id, topic_name, message, message_size);

    if (result && context.user_id) {
      if (context.scope == MQTTDS_SCOPE_FULL) {
        add_user_to_list(context.user_id, &users_tmp);
      } else if (context.scope == MQTTDS_SCOPE_USER && users_tmp.size() == 0) {
        users_tmp.push_back(context.user_id);
      }
    } else {
      context_close(&context);
      lck_lock(lck);
      _context_open = false;
      update_user_list();
      lck_unlock(lck);
    }
  }

  return result;
}

bool supla_mqtt_client_datasource::fetch(char **topic_name) {
  return fetch(topic_name, NULL, NULL);
}

bool supla_mqtt_client_datasource::is_user_queued(int user_id) {
  for (std::list<int>::iterator it = user_queue.begin(); it != user_queue.end();
       ++it) {
    if (*it == user_id) {
      return true;
    }
  }

  return false;
}

bool supla_mqtt_client_datasource::is_device_queued(int device_id) {
  for (std::list<_mqtt_ds_device_id_t>::iterator it = device_queue.begin();
       it != device_queue.end(); ++it) {
    if ((*it).device_id == device_id) {
      return true;
    }
  }

  return false;
}

bool supla_mqtt_client_datasource::is_channel_queued(int channel_id) {
  for (std::list<_mqtt_ds_channel_id_t>::iterator it = channel_queue.begin();
       it != channel_queue.end(); ++it) {
    if ((*it).channel_id == channel_id) {
      return true;
    }
  }

  return false;
}

void supla_mqtt_client_datasource::on_broker_connected(void) {
  lck_lock(lck);
  if (!all_data_expected) {
    all_data_expected = true;
    user_queue.clear();
    device_queue.clear();
    channel_queue.clear();
  }
  lck_unlock(lck);
}

void supla_mqtt_client_datasource::on_userdata_changed(int user_id) {
  lck_lock(lck);
  if (!all_data_expected && !is_user_queued(user_id)) {
    user_queue.push_back(user_id);

    for (std::list<_mqtt_ds_device_id_t>::iterator it = device_queue.begin();
         it != device_queue.end(); ++it) {
      if ((*it).user_id == user_id) {
        it = device_queue.erase(it);
        --it;
      }
    }

    for (std::list<_mqtt_ds_channel_id_t>::iterator it = channel_queue.begin();
         it != channel_queue.end(); ++it) {
      if ((*it).user_id == user_id) {
        it = channel_queue.erase(it);
        --it;
      }
    }
  }
  lck_unlock(lck);
}

void supla_mqtt_client_datasource::on_devicedata_changed(int user_id,
                                                         int device_id) {
  if (!is_mqtt_enabled(user_id)) {
    return;
  }

  lck_lock(lck);
  if (!all_data_expected && !is_user_queued(user_id) &&
      !is_device_queued(device_id)) {
    _mqtt_ds_device_id_t id = {.user_id = user_id, .device_id = device_id};
    device_queue.push_back(id);

    for (std::list<_mqtt_ds_channel_id_t>::iterator it = channel_queue.begin();
         it != channel_queue.end(); ++it) {
      if ((*it).user_id == user_id && (*it).device_id == device_id) {
        it = channel_queue.erase(it);
        --it;
      }
    }
  }
  lck_unlock(lck);
}

void supla_mqtt_client_datasource::on_channelstate_changed(int user_id,
                                                           int device_id,
                                                           int channel_id) {
  if (!is_mqtt_enabled(user_id)) {
    return;
  }

  lck_lock(lck);
  if (!all_data_expected && !is_user_queued(user_id) &&
      !is_device_queued(device_id) && !is_channel_queued(channel_id)) {
    _mqtt_ds_channel_id_t id = {
        .user_id = user_id, .device_id = device_id, .channel_id = channel_id};
    channel_queue.push_back(id);
  }
  lck_unlock(lck);
}

bool supla_mqtt_client_datasource::is_context_open(void) {
  bool result = false;
  lck_lock(lck);
  result = _context_open;
  lck_unlock(lck);

  return result;
}
