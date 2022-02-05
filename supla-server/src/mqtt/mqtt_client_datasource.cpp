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
#include "log.h"

#define WARNING_TIME_MSEC 5000

supla_mqtt_client_datasource::supla_mqtt_client_datasource(
    supla_mqtt_client_settings *settings) {
  this->lck = lck_init();
  this->context__open = false;
  this->all_data_expected = false;
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

void supla_mqtt_client_datasource::lock(void) { lck_lock(lck); }

void supla_mqtt_client_datasource::unlock(void) { lck_unlock(lck); }

void supla_mqtt_client_datasource::context_close(void) {
  if (is_context_open()) {
    context_close(&context);
    lck_lock(lck);
    context__open = false;
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
          supla_mqtt_ds_context(MQTTDS_SCOPE_DEVICE, id.user_id, id.device_id);
      result = true;
    } else if (channel_queue.size()) {
      _mqtt_ds_channel_id_t id = channel_queue.front();
      channel_queue.pop_front();

      context = supla_mqtt_ds_context(MQTTDS_SCOPE_CHANNEL_STATE, id.user_id,
                                      id.device_id, id.channel_id);

      struct timeval now;
      gettimeofday(&now, NULL);

      unsigned long long time_diff =
          ((now.tv_sec * 1000000 + now.tv_usec) -
           (id.time.tv_sec * 1000000 + id.time.tv_usec)) /
          1000;

      if (time_diff >= WARNING_TIME_MSEC) {
        supla_log(LOG_WARNING,
                  "MQTT - Context open delay: %llu msec. ChannelID: %i, Queue "
                  "size %i",
                  time_diff, id.channel_id, channel_queue.size());
      }

      result = true;
    }
  }

  lck_unlock(lck);
  return result;
}

bool supla_mqtt_client_datasource::__fetch(char **topic_name, void **message,
                                           size_t *message_size, bool *closed,
                                           bool *retain) {
  if (context_should_be_opened()) {
    bool _context__open = context_open(&context);
    lck_lock(lck);
    context__open = _context__open;
    lck_unlock(lck);
  }

  bool result = false;

  if (is_context_open()) {
    if (context.get_scope() == MQTTDS_SCOPE_FULL) {
      context.set_user_id(0);
    }
    result = _fetch(&context, topic_name, message, message_size, retain);

    if (!result) {
      context_close(&context);
      lck_lock(lck);
      context__open = false;
      lck_unlock(lck);
      *closed = true;
    }
  }

  return result;
}

bool supla_mqtt_client_datasource::fetch(char **topic_name, void **message,
                                         size_t *message_size, bool *retain) {
  bool closed = false;
  if (__fetch(topic_name, message, message_size, &closed, retain)) {
    return true;
  }

  if (closed) {
    return __fetch(topic_name, message, message_size, &closed, retain);
  }

  return false;
}

bool supla_mqtt_client_datasource::fetch(char **topic_name) {
  return fetch(topic_name, NULL, NULL, NULL);
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
  supla_mqtt_ds_context context(MQTTDS_SCOPE_FULL);

  if (!is_context_allowed(&context)) {
    return;
  }

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
  supla_mqtt_ds_context context(MQTTDS_SCOPE_USER, user_id);

  if (!is_context_allowed(&context)) {
    return;
  }

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
  supla_mqtt_ds_context context(MQTTDS_SCOPE_DEVICE, user_id, device_id);

  if (!is_context_allowed(&context)) {
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
  supla_mqtt_ds_context context(MQTTDS_SCOPE_CHANNEL_STATE, user_id, device_id,
                                channel_id);

  if (!is_context_allowed(&context)) {
    return;
  }

  lck_lock(lck);
  if (!all_data_expected && !is_user_queued(user_id) &&
      !is_device_queued(device_id) && !is_channel_queued(channel_id)) {
    struct timeval now;
    gettimeofday(&now, NULL);

    _mqtt_ds_channel_id_t id = {.user_id = user_id,
                                .device_id = device_id,
                                .channel_id = channel_id,
                                .time = now};
    channel_queue.push_back(id);
  }
  lck_unlock(lck);
}

bool supla_mqtt_client_datasource::is_context_open(void) {
  bool result = false;
  lck_lock(lck);
  result = context__open;
  lck_unlock(lck);

  return result;
}
