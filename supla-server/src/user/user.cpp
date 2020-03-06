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

#include "user.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <list>
#include "amazon/alexa.h"
#include "client.h"
#include "clientcontainer.h"
#include "database.h"
#include "device.h"
#include "devicecontainer.h"
#include "google/googlehome.h"
#include "http/httprequestqueue.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "userchannelgroups.h"

void *supla_user::user_arr = NULL;
unsigned int supla_user::client_add_metric = 0;
unsigned int supla_user::client_max_metric = 0;
unsigned int supla_user::device_add_metric = 0;
unsigned int supla_user::device_max_metric = 0;
struct timeval supla_user::metric_tv = (struct timeval){0};

// static
char supla_user::find_user_byid(void *ptr, void *UserID) {
  return ((supla_user *)ptr)->getUserID() == *(int *)UserID ? 1 : 0;
}

supla_user::supla_user(int UserID) {
  this->UserID = UserID;

  this->device_container = new supla_user_device_container();
  this->client_container = new supla_user_client_container();
  this->complex_value_functions_arr = safe_array_init();
  this->cgroups = new supla_user_channelgroups(this);
  this->amazon_alexa = new supla_amazon_alexa(this);
  this->google_home = new supla_google_home(this);
  this->connections_allowed = true;
  this->short_unique_id = NULL;
  this->long_unique_id = NULL;
  this->lck = lck_init();
  this->amazon_alexa->load();
  this->google_home->load();

  safe_array_add(supla_user::user_arr, this);
}

supla_user::~supla_user() {
  safe_array_remove(supla_user::user_arr, this);

  if (short_unique_id) {
    free(short_unique_id);
    short_unique_id = NULL;
  }

  if (long_unique_id) {
    free(long_unique_id);
    long_unique_id = NULL;
  }

  lck_free(lck);
  delete cgroups;
  delete amazon_alexa;
  delete google_home;

  compex_value_cache_clean(0);
  safe_array_free(complex_value_functions_arr);

  if (!device_container->deleteAll(10)) {
    supla_log(LOG_ERR,
              "Can't release user device container items! (TIMEOUT) UserID:%i",
              UserID);
  }
  delete device_container;

  if (!client_container->deleteAll(10)) {
    supla_log(LOG_ERR,
              "Can't release user client container items! (TIMEOUT) UserID:%i",
              UserID);
  }
  delete client_container;
}

void supla_user::init(void) { supla_user::user_arr = safe_array_init(); }

void supla_user::user_free(void) {
  supla_user *ptr = NULL;
  while ((ptr = static_cast<supla_user *>(
              safe_array_get(supla_user::user_arr, 0))) != NULL) {
    delete ptr;
  }
  safe_array_free(supla_user::user_arr);
}

int supla_user::getUserID(void) { return UserID; }

void supla_user::setUniqueId(const char shortID[], const char longID[]) {
  lck_lock(lck);

  if (!short_unique_id) {
    free(short_unique_id);
  }
  if (!long_unique_id) {
    free(long_unique_id);
  }

  short_unique_id = strndup(shortID, SHORT_UNIQUEID_MAXSIZE);
  long_unique_id = strndup(longID, LONG_UNIQUEID_MAXSIZE);
  lck_unlock(lck);
}

void supla_user::loadUniqueIDs(void) {
  char shortID[SHORT_UNIQUEID_MAXSIZE];
  char longID[LONG_UNIQUEID_MAXSIZE];

  shortID[0] = 0;
  longID[0] = 0;

  bool loaded = false;
  database *db = new database();

  if (db->connect()) {
    loaded = db->get_user_uniqueid(getUserID(), shortID, longID);
  }

  delete db;

  if (loaded) {
    shortID[SHORT_UNIQUEID_MAXSIZE - 1] = 0;
    longID[LONG_UNIQUEID_MAXSIZE - 1] = 0;
    setUniqueId(shortID, longID);
  }
}

char *supla_user::getShortUniqueID(void) {
  char *result = NULL;

  lck_lock(lck);

  if (!short_unique_id) {
    loadUniqueIDs();
  }

  if (short_unique_id) {
    result = strdup(short_unique_id);
  }
  lck_unlock(lck);

  return result;
}

char *supla_user::getLongUniqueID(void) {
  char *result = NULL;

  lck_lock(lck);

  if (!long_unique_id) {
    loadUniqueIDs();
  }

  if (long_unique_id) {
    result = strdup(long_unique_id);
  }
  lck_unlock(lck);

  return result;
}

// static
int supla_user::user_count(void) {
  return safe_array_count(supla_user::user_arr);
}

// static
supla_user *supla_user::get_user(int idx) {
  return (supla_user *)safe_array_get(supla_user::user_arr, idx);
}

void supla_user::moveDeviceToTrash(supla_device *device) {
  device_container->moveToTrash(device);
}

void supla_user::moveClientToTrash(supla_client *client) {
  client_container->moveToTrash(client);
}

void supla_user::emptyTrash(void) {
  device_container->emptyTrash();
  client_container->emptyTrash();
}

bool supla_user::getClientName(int ClientID, char *buffer, int size) {
  if (size < 1) return false;

  buffer[0] = 0;

  supla_client *client = client_container->findByID(ClientID);

  if (client) {
    client->getName(buffer, size);
    client->releasePtr();
  }

  return client != NULL;
}

bool supla_user::isSuperUserAuthorized(int ClientID) {
  bool result = false;

  supla_client *client = client_container->findByID(ClientID);

  if (client) {
    result = client->is_superuser_authorized();
    client->releasePtr();
  }

  return result;
}

supla_user *supla_user::find(int UserID, bool create) {
  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user == NULL) user = new supla_user(UserID);

  safe_array_unlock(supla_user::user_arr);

  return user;
}

supla_user *supla_user::add_device(supla_device *device, int UserID) {
  supla_user *user = find(UserID, true);

  if (device && device->getID()) {
    user->compex_value_cache_clean(device->getID());
  }

  if (!user->device_container->exists(device)) {
    char GUID[SUPLA_GUID_SIZE];
    memset(GUID, 0, SUPLA_GUID_SIZE);
    device->getGUID(GUID);

    supla_device *_device = user->device_container->findByGUID(GUID);
    user->device_container->addToList(device);

    if (_device != NULL) {
      _device->terminate();
      _device->releasePtr();
    }

    safe_array_lock(supla_user::user_arr);
    device_add_metric++;

    unsigned int dc = total_cd_count(false);
    if (dc > supla_user::device_max_metric) {
      supla_user::device_max_metric = dc;
    }
    safe_array_unlock(supla_user::user_arr);
  }

  return user;
}

supla_user *supla_user::add_client(supla_client *client, int UserID) {
  supla_user *user = find(UserID, true);

  if (!user->client_container->exists(client)) {
    char GUID[SUPLA_GUID_SIZE];
    memset(GUID, 0, SUPLA_GUID_SIZE);
    client->getGUID(GUID);

    supla_client *_client = user->client_container->findByGUID(GUID);
    user->client_container->addToList(client);

    if (_client != NULL) {
      _client->terminate();
      _client->releasePtr();
    }

    safe_array_lock(supla_user::user_arr);
    client_add_metric++;

    unsigned int cc = total_cd_count(true);
    if (cc > supla_user::client_max_metric) {
      supla_user::client_max_metric = cc;
    }
    safe_array_unlock(supla_user::user_arr);
  }

  return user;
}

bool supla_user::is_client_online(int ClientID) {
  supla_client *client = client_container->findByID(ClientID);
  if (client) {
    client->releasePtr();
    return true;
  }

  return false;
}

bool supla_user::is_device_online(int DeviceID) {
  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    device->releasePtr();
    return true;
  }
  return false;
}

// static
bool supla_user::is_client_online(int UserID, int ClientID) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user && user->is_client_online(ClientID) == true) result = true;

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
bool supla_user::is_device_online(int UserID, int DeviceID) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user && user->is_device_online(DeviceID) == true) result = true;

  safe_array_unlock(supla_user::user_arr);

  return result;
}

bool supla_user::get_channel_double_value(int DeviceID, int ChannelID,
                                          double *Value, char Type) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);
  if (device != NULL) {
    switch (Type) {
      case 0:
        result = device->get_channel_double_value(ChannelID, Value) == 1;
        break;
      case 1:
        result = device->get_channel_temperature_value(ChannelID, Value) == 1;
        break;
      case 2:
        result = device->get_channel_humidity_value(ChannelID, Value) == 1;
        break;
    }
    device->releasePtr();
  }

  return result;
}

bool supla_user::get_channel_double_value(int DeviceID, int ChannelID,
                                          double *Value) {
  return get_channel_double_value(DeviceID, ChannelID, Value, 0);
}

bool supla_user::get_channel_temperature_value(int DeviceID, int ChannelID,
                                               double *Value) {
  return get_channel_double_value(DeviceID, ChannelID, Value, 1);
}

bool supla_user::get_channel_humidity_value(int DeviceID, int ChannelID,
                                            double *Value) {
  return get_channel_double_value(DeviceID, ChannelID, Value, 2);
}

bool supla_user::get_channel_char_value(int DeviceID, int ChannelID,
                                        char *Value) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);

  if (device != NULL) {
    result = device->get_channel_char_value(ChannelID, Value) == 1;
    device->releasePtr();
  }

  return result;
}

bool supla_user::get_channel_rgbw_value(int DeviceID, int ChannelID, int *color,
                                        char *color_brightness,
                                        char *brightness, char *on_off) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);
  if (device != NULL) {
    result = device->get_channel_rgbw_value(ChannelID, color, color_brightness,
                                            brightness, on_off) == 1;
    device->releasePtr();
  }

  return result;
}

// static
bool supla_user::get_channel_double_value(int UserID, int DeviceID,
                                          int ChannelID, double *Value,
                                          char Type) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    switch (Type) {
      case 0:
        result =
            user->get_channel_double_value(DeviceID, ChannelID, Value) == true;
        break;
      case 1:
        result = user->get_channel_temperature_value(DeviceID, ChannelID,
                                                     Value) == true;
        break;
      case 2:
        result = user->get_channel_humidity_value(DeviceID, ChannelID, Value) ==
                 true;
        break;
    }
  }

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
bool supla_user::get_channel_double_value(int UserID, int DeviceID,
                                          int ChannelID, double *Value) {
  return get_channel_double_value(UserID, DeviceID, ChannelID, Value, 0);
}

// static
bool supla_user::get_channel_temperature_value(int UserID, int DeviceID,
                                               int ChannelID, double *Value) {
  return get_channel_double_value(UserID, DeviceID, ChannelID, Value, 1);
}

// static
bool supla_user::get_channel_humidity_value(int UserID, int DeviceID,
                                            int ChannelID, double *Value) {
  return get_channel_double_value(UserID, DeviceID, ChannelID, Value, 2);
}

// static
bool supla_user::get_channel_char_value(int UserID, int DeviceID, int ChannelID,
                                        char *Value) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    result = user->get_channel_char_value(DeviceID, ChannelID, Value) == true;
  }

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
bool supla_user::get_channel_rgbw_value(int UserID, int DeviceID, int ChannelID,
                                        int *color, char *color_brightness,
                                        char *brightness, char *on_off) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    result = user->get_channel_rgbw_value(DeviceID, ChannelID, color,
                                          color_brightness, brightness,
                                          on_off) == true;
  }

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
supla_channel_electricity_measurement *supla_user::get_electricity_measurement(
    int UserID, int DeviceID, int ChannelID) {
  supla_channel_electricity_measurement *result = NULL;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    result = user->get_electricity_measurement(DeviceID, ChannelID);
  }

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
supla_channel_ic_measurement *supla_user::supla_user::get_ic_measurement(
    int UserID, int DeviceID, int ChannelID) {
  supla_channel_ic_measurement *result = NULL;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    result = user->get_ic_measurement(DeviceID, ChannelID);
  }

  safe_array_unlock(supla_user::user_arr);

  return result;
}

bool supla_user::get_channel_value(int DeviceID, int ChannelID,
                                   TSuplaChannelValue *value, char *online) {
  bool result = false;
  memset(value, 0, sizeof(TSuplaChannelValue));
  *online = 0;

  supla_device *related_device = NULL;
  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    *online = 1;
    result = device->get_channel_value(ChannelID, value->value);

    if (result) {
      std::list<int> related_list = device->related_channel(ChannelID);

      std::list<int>::iterator it = related_list.begin();

      if (related_list.size() == 1 && *it > 0) {
        related_device = device_container->findByChannelID(*it);
        if (related_device) {
          related_device->get_channel_value(*it, value->sub_value);
          related_device->releasePtr();
          related_device = NULL;
        }
      } else if (related_list.size() > 1) {
        // For more than one sub-channel
        // copy only first byte of sub_value
        // At this version only SENSORNO/SENSORNC are supported.
        // See supla_device_channel::related_channel
        char sub_value[SUPLA_CHANNELVALUE_SIZE];
        int n = 0;
        do {
          if (*it > 0) {
            related_device = device_container->findByChannelID(*it);
            if (related_device) {
              related_device->get_channel_value(*it, sub_value);
              value->sub_value[n] = sub_value[0];
              related_device->releasePtr();
              related_device = NULL;
            }
          }
          n++;
          it++;
        } while (it != related_list.end() && n < SUPLA_CHANNELVALUE_SIZE);
      }
    }

    device->releasePtr();
  }

  return result;
}

bool supla_user::get_channel_extendedvalue(int DeviceID, int ChannelID,
                                           TSuplaChannelExtendedValue *value) {
  bool result = false;
  memset(value, 0, sizeof(TSuplaChannelExtendedValue));

  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    result = device->get_channel_extendedvalue(ChannelID, value);
    device->releasePtr();
  }

  return result;
}

// static
bool supla_user::set_device_channel_char_value(
    int UserID, int SenderID, int DeviceID, int ChannelID, const char value,
    event_source_type eventSourceType, char *AlexaCorrelationToken,
    char *GoogleRequestId) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    if (eventSourceType > 0) {
      supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
          user, DeviceID, ChannelID, eventSourceType, AlexaCorrelationToken,
          GoogleRequestId);
    }

    result = user->set_device_channel_char_value(SenderID, DeviceID, ChannelID,
                                                 value) == true;
  }

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
bool supla_user::set_device_channel_rgbw_value(
    int UserID, int SenderID, int DeviceID, int ChannelID, int color,
    char color_brightness, char brightness, char on_off,
    event_source_type eventSourceType, char *AlexaCorrelationToken,
    char *GoogleRequestId) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    if (eventSourceType > 0) {
      supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
          user, DeviceID, ChannelID, eventSourceType, AlexaCorrelationToken,
          GoogleRequestId);
    }

    result = user->set_device_channel_rgbw_value(SenderID, DeviceID, ChannelID,
                                                 color, color_brightness,
                                                 brightness, on_off) == true;
  }

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
bool supla_user::set_channelgroup_char_value(int UserID, int GroupID,
                                             const char value) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) result = user->set_channelgroup_char_value(GroupID, value) == true;

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
bool supla_user::set_channelgroup_rgbw_value(int UserID, int GroupID, int color,
                                             char color_brightness,
                                             char brightness, char on_off) {
  bool result = false;

  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user)
    result = user->set_channelgroup_rgbw_value(GroupID, color, color_brightness,
                                               brightness, on_off) == true;

  safe_array_unlock(supla_user::user_arr);

  return result;
}

// static
void supla_user::on_amazon_alexa_credentials_changed(int UserID) {
  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    user->amazonAlexa()->on_credentials_changed();
  }

  safe_array_unlock(supla_user::user_arr);
}

// static
void supla_user::on_google_home_credentials_changed(int UserID) {
  safe_array_lock(supla_user::user_arr);
  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    user->googleHome()->on_credentials_changed();
  }

  safe_array_unlock(supla_user::user_arr);
}

// static
void supla_user::on_device_deleted(int UserID,
                                   event_source_type eventSourceType) {
  safe_array_lock(supla_user::user_arr);
  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_byid, &UserID);

  if (user) {
    supla_http_request_queue::getInstance()->onDeviceDeletedEvent(
        user, 0, eventSourceType);
  }

  safe_array_unlock(supla_user::user_arr);
}

// static
unsigned int supla_user::total_cd_count(bool client) {
  unsigned int result = 0;
  supla_user *user = NULL;

  safe_array_lock(supla_user::user_arr);

  for (int a = 0; a < safe_array_count(user_arr); a++) {
    if (NULL != (user = (supla_user *)safe_array_get(user_arr, a))) {
      result += client ? user->client_container->count()
                       : user->device_container->count();
    }
  }

  safe_array_unlock(supla_user::user_arr);
  return result;
}

// static
void supla_user::print_metrics(int min_interval_sec) {
  if (min_interval_sec > 0) {
    struct timeval now;
    gettimeofday(&now, NULL);
    if (supla_user::metric_tv.tv_sec == 0) {
      supla_user::metric_tv = now;
    }
    if (now.tv_sec - supla_user::metric_tv.tv_sec < min_interval_sec) {
      return;
    }
    supla_user::metric_tv = now;
  }

  supla_user *user = NULL;

  unsigned int user_count = 0;
  unsigned int client_count = 0;
  unsigned int client_trash = 0;
  unsigned int device_count = 0;
  unsigned int device_trash = 0;

  safe_array_lock(supla_user::user_arr);

  user_count = safe_array_count(user_arr);

  for (unsigned int a = 0; a < user_count; a++) {
    if (NULL != (user = (supla_user *)safe_array_get(user_arr, a))) {
      client_count += user->client_container->count();
      client_trash += user->client_container->trashCount();
      device_count += user->device_container->count();
      device_trash += user->device_container->trashCount();
    }
  }

  safe_array_unlock(supla_user::user_arr);

  supla_log(LOG_INFO,
            "METRICS: USER[COUNT:%u] CLIENT[ADD:%u MAX:%u COUNT:%u TRASH:%u] "
            "DEVICE[ADD:%u MAX:%u COUNT:%u TRASH:%u]",
            user_count, supla_user::client_add_metric,
            supla_user::client_max_metric, client_count, client_trash,
            supla_user::device_add_metric, supla_user::device_max_metric,
            device_count, device_trash);
}

void supla_user::on_device_added(int DeviceID,
                                 event_source_type eventSourceType) {
  supla_http_request_queue::getInstance()->onDeviceAddedEvent(this, DeviceID,
                                                              eventSourceType);
}

bool supla_user::set_device_channel_value(
    event_source_type eventSourceType, int SenderID, int DeviceID,
    int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
        this, DeviceID, ChannelID, eventSourceType);

    device->set_device_channel_value(SenderID, ChannelID, value);
    device->releasePtr();
  }

  return result;
}

bool supla_user::set_device_channel_char_value(int SenderID, int DeviceID,
                                               int ChannelID,
                                               const char value) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    result = device->set_device_channel_char_value(SenderID, ChannelID, value);
    device->releasePtr();
  }

  return result;
}

bool supla_user::set_device_channel_rgbw_value(int SenderID, int DeviceID,
                                               int ChannelID, int color,
                                               char color_brightness,
                                               char brightness, char on_off) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    result = device->set_device_channel_rgbw_value(
        SenderID, ChannelID, color, color_brightness, brightness, on_off);
    device->releasePtr();
  }

  return result;
}

bool supla_user::set_channelgroup_char_value(int GroupID, const char value) {
  return cgroups->set_char_value(GroupID, value);
}

bool supla_user::set_channelgroup_rgbw_value(int GroupID, int color,
                                             char color_brightness,
                                             char brightness, char on_off) {
  return cgroups->set_rgbw_value(GroupID, color, color_brightness, brightness,
                                 on_off);
}

void supla_user::update_client_device_channels(int LocationID, int DeviceID) {
  {
    supla_client *client;

    for (int a = 0; a < client_container->count(); a++)
      if (NULL != (client = client_container->get(a))) {
        client->update_device_channels(LocationID, DeviceID);
        client->releasePtr();
      }
  }
}

void supla_user::on_channel_value_changed(event_source_type eventSourceType,
                                          int DeviceId, int ChannelId,
                                          bool Extended) {
  if (!Extended && DeviceId && ChannelId && eventSourceType != EST_UNKNOWN) {
    supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
        this, DeviceId, ChannelId, eventSourceType);
  }

  std::list<channel_address> ca_list;

  if (Extended) {
    {
      supla_client *client;

      for (int a = 0; a < client_container->count(); a++)
        if (NULL != (client = client_container->get(a))) {
          client->on_channel_value_changed(DeviceId, ChannelId, true);
          client->releasePtr();
        }
    }
    return;
  }

  supla_device *device = device_container->findByID(DeviceId);
  if (device) {
    std::list<int> master_list = device->master_channel(ChannelId);

    device->releasePtr();
    device = NULL;

    for (std::list<int>::iterator it = master_list.begin();
         it != master_list.end(); it++) {
      device = device_container->findByChannelID(*it);

      if (device) {
        ca_list.push_back(channel_address(device->getID(), *it));
        device->releasePtr();
        device = NULL;
      }
    }
  }

  if (ca_list.empty()) {
    ca_list.push_back(channel_address(DeviceId, ChannelId));
  }

  supla_client *client;

  for (int a = 0; a < client_container->count(); a++)
    if (NULL != (client = client_container->get(a))) {
      for (std::list<channel_address>::iterator it = ca_list.begin();
           it != ca_list.end(); it++) {
        client->on_channel_value_changed(it->getDeviceId(), it->getChannelId());
      }
      client->releasePtr();
    }
}

void supla_user::on_channel_become_online(int DeviceId, int ChannelId) {
  supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
      this, DeviceId, ChannelId, EST_DEVICE);
}

void supla_user::call_event(TSC_SuplaEvent *event) {
  supla_client *client;

  for (int a = 0; a < client_container->count(); a++)
    if (NULL != (client = client_container->get(a))) {
      client->call_event(event);
      client->releasePtr();
    }
}

void supla_user::get_temp_and_humidity(void *tarr) {
  int a;
  supla_device *device;

  for (a = 0; a < device_container->count(); a++) {
    if (NULL != (device = device_container->get(a))) {
      device->get_temp_and_humidity(tarr);
      device->releasePtr();
    }
  }
}

void supla_user::get_electricity_measurements(void *emarr) {
  int a;
  supla_device *device;

  for (a = 0; a < device_container->count(); a++) {
    if (NULL != (device = device_container->get(a))) {
      device->get_electricity_measurements(emarr);
      device->releasePtr();
    }
  }
}

supla_channel_electricity_measurement *supla_user::get_electricity_measurement(
    int DeviceID, int ChannelID) {
  supla_channel_electricity_measurement *result = NULL;

  supla_device *device = device_container->findByID(DeviceID);
  if (device != NULL) {
    result = device->get_electricity_measurement(ChannelID);
    device->releasePtr();
  }

  return result;
}

void supla_user::get_ic_measurements(void *icarr) {
  int a;
  supla_device *device;

  for (a = 0; a < device_container->count(); a++) {
    if (NULL != (device = device_container->get(a))) {
      device->get_ic_measurements(icarr);
      device->releasePtr();
    }
  }
}

supla_channel_ic_measurement *supla_user::get_ic_measurement(int DeviceID,
                                                             int ChannelID) {
  supla_channel_ic_measurement *result = NULL;

  supla_device *device = device_container->findByID(DeviceID);
  if (device != NULL) {
    result = device->get_ic_measurement(ChannelID);
    device->releasePtr();
  }

  return result;
}

void supla_user::get_thermostat_measurements(void *tharr) {
  int a;
  supla_device *device;

  for (a = 0; a < device_container->count(); a++) {
    if (NULL != (device = device_container->get(a))) {
      device->get_thermostat_measurements(tharr);
      device->releasePtr();
    }
  }
}

bool supla_user::device_calcfg_request(int SenderID, int DeviceId,
                                       int ChannelId,
                                       TCS_DeviceCalCfgRequest_B *request) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceId);
  if (device) {
    result = device->calcfg_request(
        SenderID, ChannelId,
        SenderID > 0 ? isSuperUserAuthorized(SenderID) : false, request);
    device->releasePtr();
  }

  return result;
}

void supla_user::on_device_calcfg_result(int ChannelID,
                                         TDS_DeviceCalCfgResult *result) {
  if (result == NULL) return;

  supla_client *client = client_container->findByID(result->ReceiverID);

  if (client) {
    client->on_device_calcfg_result(ChannelID, result);
    client->releasePtr();
  }
}

void supla_user::reconnect(event_source_type eventSourceType) {
  int a;

  cgroups->load();  // load == reload

  supla_device *device;
  std::list<cdbase *> cdb;

  for (a = 0; a < device_container->count(); a++)
    if (NULL != (device = device_container->get(a))) {
      cdb.push_back(device);
    }

  supla_client *client;

  for (a = 0; a < client_container->count(); a++)
    if (NULL != (client = client_container->get(a))) {
      cdb.push_back(client);
    }

  for (std::list<cdbase *>::iterator it = cdb.begin(); it != cdb.end(); it++) {
    (*it)->terminate();
    (*it)->releasePtr();
  }

  supla_http_request_queue::getInstance()->onUserReconnectEvent(
      this, eventSourceType);
}

bool supla_user::client_reconnect(int ClientID) {
  bool result = false;

  supla_client *client = client_container->findByID(ClientID);

  if (client) {
    client->terminate();
    client->releasePtr();
    result = true;
  }

  return result;
}

// static
bool supla_user::reconnect(int UserID, event_source_type eventSourceType) {
  supla_user *user = find(UserID, true);

  if (user) {
    user->reconnect(eventSourceType);
    // cppcheck-suppress memleak
    return true;
  }

  return false;
}

// static
bool supla_user::client_reconnect(int UserID, int ClientID) {
  supla_user *user = find(UserID, true);

  if (user) {
    return user->client_reconnect(ClientID);
  }

  return false;
}

void supla_user::compex_value_cache_clean(int DeviceId) {
  safe_array_lock(complex_value_functions_arr);

  for (int a = 0; a < safe_array_count(complex_value_functions_arr); a++) {
    channel_function_t *fnc = static_cast<channel_function_t *>(
        safe_array_get(complex_value_functions_arr, a));
    if (fnc && (DeviceId == 0 || fnc->deviceId == DeviceId)) {
      delete fnc;
      safe_array_delete(complex_value_functions_arr, a);
      a--;
    }
  }

  safe_array_unlock(complex_value_functions_arr);
}

channel_function_t supla_user::compex_value_cache_get_function(
    int ChannelID, channel_function_t **_fnc) {
  channel_function_t result;
  memset(&result, 0, sizeof(channel_function_t));

  safe_array_lock(complex_value_functions_arr);

  for (int a = 0; a < safe_array_count(complex_value_functions_arr); a++) {
    channel_function_t *fnc = static_cast<channel_function_t *>(
        safe_array_get(complex_value_functions_arr, a));
    if (fnc && fnc->channelId == ChannelID) {
      if (_fnc) {
        *_fnc = fnc;
      }
      result = *fnc;
      break;
    }
  }

  safe_array_unlock(complex_value_functions_arr);
  return result;
}

void supla_user::compex_value_cache_update_function(int DeviceId, int ChannelID,
                                                    int Function,
                                                    bool channel_is_hidden) {
  if (!Function || !DeviceId || !ChannelID) return;
  safe_array_lock(complex_value_functions_arr);

  channel_function_t *fnc = NULL;
  if (compex_value_cache_get_function(ChannelID, &fnc).function) {
    if (fnc) {
      fnc->deviceId = DeviceId;
      fnc->function = Function;
      fnc->channel_is_hidden = channel_is_hidden;
    }
  } else {
    fnc = new channel_function_t;
    fnc->deviceId = DeviceId;
    fnc->channelId = ChannelID;
    fnc->function = Function;
    fnc->channel_is_hidden = channel_is_hidden;

    safe_array_add(complex_value_functions_arr, fnc);
  }
  safe_array_unlock(complex_value_functions_arr);
}

channel_complex_value supla_user::get_channel_complex_value(int DeviceId,
                                                            int ChannelID) {
  channel_complex_value value;
  memset(&value, 0, sizeof(channel_complex_value));

  supla_device *device = device_container->findByChannelID(ChannelID);
  if (device == NULL) {
    channel_function_t f = compex_value_cache_get_function(ChannelID);
    value.function = f.function;
    value.hidden_channel = f.channel_is_hidden;
  } else {
    device->get_channel_complex_value(&value, ChannelID);
    if (value.function) {
      compex_value_cache_update_function(device->getID(), ChannelID,
                                         value.function, value.hidden_channel);
    }
    device->releasePtr();
  }

  return value;
}

supla_amazon_alexa *supla_user::amazonAlexa(void) { return amazon_alexa; }

supla_google_home *supla_user::googleHome(void) { return google_home; }
