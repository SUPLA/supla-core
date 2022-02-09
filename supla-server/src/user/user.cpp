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

#include <amazon/alexacredentials.h>
#include <google/googlehomecredentials.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <list>

#include "client.h"
#include "clientcontainer.h"
#include "database.h"
#include "device.h"
#include "devicecontainer.h"
#include "http/httprequestqueue.h"
#include "lck.h"
#include "log.h"
#include "mqtt/mqtt_client_suite.h"
#include "safearray.h"
#include "serverstatus.h"
#include "userchannelgroups.h"

void *supla_user::user_arr = NULL;
unsigned int supla_user::client_add_metric = 0;
unsigned int supla_user::client_max_metric = 0;
unsigned int supla_user::device_add_metric = 0;
unsigned int supla_user::device_max_metric = 0;
struct timeval supla_user::metric_tv = (struct timeval){0};

// static
char supla_user::find_user_by_id(void *ptr, void *UserID) {
  return ((supla_user *)ptr)->getUserID() == *(int *)UserID ? 1 : 0;
}

// static
char supla_user::find_user_by_suid(void *ptr, void *suid) {
  const char *_suid = ((supla_user *)ptr)->getShortUniqueID();
  if (_suid == NULL) {
    return 0;
  }

  return strncmp((char *)suid, _suid, SHORT_UNIQUEID_MAXSIZE) == 0 ? 1 : 0;
}

void supla_user::user_init(int UserID, const char *short_unique_id,
                           const char *long_unique_id) {
  this->UserID = UserID;

  this->device_container = new supla_user_device_container();
  this->client_container = new supla_user_client_container();
  this->complex_value_functions_arr = safe_array_init();
  this->cgroups = new supla_user_channelgroups(this);
  this->amazon_alexa_credentials = new supla_amazon_alexa_credentials(this);
  this->google_home_credentials = new supla_google_home_credentials(this);
  this->state_webhook_credentials = new supla_state_webhook_credentials(this);
  this->connections_allowed = true;
  this->short_unique_id =
      short_unique_id ? strndup(short_unique_id, SHORT_UNIQUEID_MAXSIZE) : NULL;
  this->long_unique_id =
      long_unique_id ? strndup(long_unique_id, LONG_UNIQUEID_MAXSIZE) : NULL;
  this->lck = lck_init();
  this->amazon_alexa_credentials->load();
  this->google_home_credentials->load();
  this->state_webhook_credentials->load();

  safe_array_add(supla_user::user_arr, this);
}

supla_user::supla_user(int UserID) { user_init(UserID, NULL, NULL); }  // NOLINT

supla_user::supla_user(int UserID, const char *short_unique_id,
                       const char *long_unique_id) {
  user_init(UserID, short_unique_id, long_unique_id);
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
  delete amazon_alexa_credentials;
  delete google_home_credentials;
  delete state_webhook_credentials;

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

char *supla_user::getUniqueID(char **id, bool longid) {
  char *result = NULL;

  lck_lock(lck);

  if (!(*id)) {
    *id = (char *)calloc(
        longid ? LONG_UNIQUEID_MAXSIZE : SHORT_UNIQUEID_MAXSIZE, sizeof(char));

    if (*id) {
      database *db = new database();

      if (!db->connect() || !db->get_user_uniqueid(getUserID(), *id, longid)) {
        free(*id);
        *id = NULL;
      }

      delete db;
    }
  }

  result = *id;

  lck_unlock(lck);

  return result;
}

const char *supla_user::getShortUniqueID(void) {
  return getUniqueID(&short_unique_id, false);
}

const char *supla_user::getLongUniqueID(void) {
  return getUniqueID(&long_unique_id, true);
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

// static
supla_user *supla_user::find(int UserID, bool create) {
  safe_array_lock(supla_user::user_arr);

  supla_user *user =
      (supla_user *)safe_array_findcnd(user_arr, find_user_by_id, &UserID);

  if (user == NULL && create) user = new supla_user(UserID);

  safe_array_unlock(supla_user::user_arr);

  return user;
}

// static
supla_user *supla_user::find_by_suid(const char *suid) {
  if (suid == NULL || suid[0] == 0) {
    return NULL;
  }

  safe_array_lock(supla_user::user_arr);

  supla_user *user = (supla_user *)safe_array_findcnd(
      user_arr, find_user_by_suid, (void *)suid);

  safe_array_unlock(supla_user::user_arr);

  return user;
}

// static
int supla_user::suid_to_user_id(const char *suid, bool use_database) {
  supla_user *user = find_by_suid(suid);
  if (user) {
    return user->getUserID();
  }

  int result = 0;

  if (use_database) {
    database *db = new database();

    if (db->connect() == true) {
      result = db->get_user_id_by_suid(suid);
    }

    delete db;
  }

  return result;
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
    safe_array_unlock(supla_user::user_arr);

    unsigned int dc = total_cd_count(false);

    safe_array_lock(supla_user::user_arr);
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
    safe_array_unlock(supla_user::user_arr);

    unsigned int cc = total_cd_count(true);

    safe_array_lock(supla_user::user_arr);
    if (cc > supla_user::client_max_metric) {
      supla_user::client_max_metric = cc;
    }
    safe_array_unlock(supla_user::user_arr);
  }

  return user;
}

void supla_user::access_client(
    int ClientID, std::function<void(supla_client *client)> on_client) {
  supla_client *client = client_container->findByID(ClientID);
  if (client) {
    on_client(client);
    client->releasePtr();
  }
}

bool supla_user::is_client_online(int ClientID) {
  bool result = false;
  access_client(ClientID,
                [&result](supla_client *client) -> void { result = true; });
  return result;
}

bool supla_user::is_device_online(int DeviceID) {
  bool result = false;
  access_device(DeviceID, 0,
                [&result](supla_device *device) -> void { result = true; });
  return result;
}

bool supla_user::is_channel_online(int DeviceID, int ChannelID) {
  bool result = false;
  access_device(DeviceID, 0,
                [&result, ChannelID](supla_device *device) -> void {
                  result = device->get_channels()->is_channel_online(ChannelID);
                });
  return result;
}

// static
bool supla_user::is_client_online(int UserID, int ClientID) {
  bool result = false;

  supla_user *user = supla_user::find(UserID, false);
  if (user && user->is_client_online(ClientID) == true) {
    result = true;
  }

  return result;
}

// static
bool supla_user::is_device_online(int UserID, int DeviceID) {
  bool result = false;

  supla_user *user = supla_user::find(UserID, false);

  if (user && user->is_device_online(DeviceID) == true) {
    result = true;
  }

  return result;
}

// static
bool supla_user::is_channel_online(int UserID, int DeviceID, int ChannelID) {
  bool result = false;

  supla_user *user = supla_user::find(UserID, false);

  if (user && user->is_channel_online(DeviceID, ChannelID) == true) {
    result = true;
  }

  return result;
}

// static
supla_device *supla_user::get_device(int UserID, int DeviceID) {
  supla_user *user = find(UserID, false);
  if (user) {
    return user->get_device(DeviceID);
  }

  return NULL;
}

supla_device *supla_user::get_device(int DeviceID) {
  return device_container->findByID(DeviceID);
}

void supla_user::access_device(
    int DeviceId, int ChannelId,
    std::function<void(supla_device *device)> on_device) {
  if (DeviceId || ChannelId) {
    supla_device *device = DeviceId
                               ? device_container->findByID(DeviceId)
                               : device_container->findByChannelID(ChannelId);
    if (device) {
      on_device(device);
      device->releasePtr();
    }
  }
}

// static
void supla_user::access_device(
    int UserID, int DeviceId, int ChannelId,
    std::function<void(supla_device *device)> on_device) {
  supla_user *user = find(UserID, false);
  if (user) {
    user->access_device(DeviceId, ChannelId, on_device);
  }
}

bool supla_user::get_channel_double_value(int DeviceID, int ChannelID,
                                          double *Value, char Type) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);
  if (device != NULL) {
    supla_device_channels *channels = device->get_channels();
    switch (Type) {
      case 0:
        result = channels->get_channel_double_value(ChannelID, Value) == 1;
        break;
      case 1:
        result = channels->get_channel_temperature_value(ChannelID, Value) == 1;
        break;
      case 2:
        result = channels->get_channel_humidity_value(ChannelID, Value) == 1;
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
    result =
        device->get_channels()->get_channel_char_value(ChannelID, Value) == 1;
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
    result = device->get_channels()->get_channel_rgbw_value(
                 ChannelID, color, color_brightness, brightness, on_off) == 1;
    device->releasePtr();
  }

  return result;
}

bool supla_user::get_channel_valve_value(int DeviceID, int ChannelID,
                                         TValve_Value *Value) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);

  if (device != NULL) {
    result =
        device->get_channels()->get_channel_valve_value(ChannelID, Value) == 1;
    device->releasePtr();
  }

  return result;
}

// static
bool supla_user::get_channel_double_value(int UserID, int DeviceID,
                                          int ChannelID, double *Value,
                                          char Type) {
  bool result = false;

  supla_user *user = supla_user::find(UserID, false);

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

  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    result = user->get_channel_char_value(DeviceID, ChannelID, Value) == true;
  }

  return result;
}

// static
bool supla_user::get_channel_rgbw_value(int UserID, int DeviceID, int ChannelID,
                                        int *color, char *color_brightness,
                                        char *brightness, char *on_off) {
  bool result = false;

  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    result = user->get_channel_rgbw_value(DeviceID, ChannelID, color,
                                          color_brightness, brightness,
                                          on_off) == true;
  }

  return result;
}

// static
bool supla_user::get_channel_valve_value(int UserID, int DeviceID,
                                         int ChannelID, TValve_Value *Value) {
  bool result = false;

  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    result = user->get_channel_valve_value(DeviceID, ChannelID, Value) == true;
  }

  return result;
}

supla_device *supla_user::device_by_channelid(int ChannelID) {
  return device_container->findByChannelID(ChannelID);
}

bool supla_user::get_channel_value(int DeviceID, int ChannelID,
                                   char value[SUPLA_CHANNELVALUE_SIZE],
                                   char sub_value[SUPLA_CHANNELVALUE_SIZE],
                                   char *sub_value_type, char *online,
                                   unsigned _supla_int_t *validity_time_sec,
                                   bool for_client) {
  bool result = false;
  memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
  memset(sub_value, 0, SUPLA_CHANNELVALUE_SIZE);
  if (online) {
    *online = 0;
  }

  supla_device *related_device = NULL;
  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    result = device->get_channels()->get_channel_value(
        ChannelID, value, online, validity_time_sec, for_client);

    if (result) {
      std::list<int> related_list =
          device->get_channels()->related_channel(ChannelID);

      std::list<int>::iterator it = related_list.begin();
      bool sub_value_exists = false;
      char sub_channel_online = 0;

      if (related_list.size() == 1 && *it > 0) {
        related_device = device_container->findByChannelID(*it);
        if (related_device) {
          if (related_device->get_channels()->get_channel_value(
                  *it, sub_value, &sub_channel_online, NULL, for_client) &&
              sub_channel_online) {
            sub_value_exists = true;
          } else {
            memset(sub_value, 0, SUPLA_CHANNELVALUE_SIZE);
          }
          related_device->releasePtr();
          related_device = NULL;
        }
      } else if (related_list.size() > 1) {
        // For more than one sub-channel
        // copy only first byte of sub_value
        // At this version only SENSORNO/SENSORNC are supported.
        // See supla_device_channel::related_channel
        char _sub_value[SUPLA_CHANNELVALUE_SIZE];
        int n = 0;
        do {
          if (*it > 0) {
            related_device = device_container->findByChannelID(*it);
            if (related_device) {
              sub_channel_online = false;
              if (related_device->get_channels()->get_channel_value(
                      *it, _sub_value, &sub_channel_online, NULL, for_client) &&
                  sub_channel_online) {
                sub_value[n] = _sub_value[0];
                sub_value_exists = true;
              }

              related_device->releasePtr();
              related_device = NULL;
            }
          }
          n++;
          it++;
        } while (it != related_list.end() && n < SUPLA_CHANNELVALUE_SIZE);
      }

      if (sub_value_type) {
        it = related_list.begin();
        *sub_value_type = SUBV_TYPE_NOT_SET_OR_OFFLINE;

        if (sub_value_exists && related_list.size() > 0 && *it > 0) {
          related_device = device_container->findByChannelID(*it);
          if (related_device) {
            int rel_channel_func =
                related_device->get_channels()->get_channel_func(*it);

            related_device->releasePtr();
            related_device = NULL;

            switch (rel_channel_func) {
              case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
                *sub_value_type = SUBV_TYPE_SENSOR;
                break;
              case SUPLA_CHANNELFNC_ELECTRICITY_METER:
                *sub_value_type = SUBV_TYPE_ELECTRICITY_MEASUREMENTS;
                break;
              case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
              case SUPLA_CHANNELFNC_IC_GAS_METER:
              case SUPLA_CHANNELFNC_IC_WATER_METER:
              case SUPLA_CHANNELFNC_IC_HEAT_METER:
                *sub_value_type = SUBV_TYPE_IC_MEASUREMENTS;
                break;
            }
          }
        }
      }
    }

    device->releasePtr();
  }

  return result;
}

// static
void supla_user::on_amazon_alexa_credentials_changed(int UserID) {
  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    user->amazonAlexaCredentials()->on_credentials_changed();
  }
}

// static
void supla_user::on_google_home_credentials_changed(int UserID) {
  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    user->googleHomeCredentials()->on_credentials_changed();
  }
}

// static
void supla_user::on_state_webhook_changed(int UserID) {
  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    user->stateWebhookCredentials()->on_credentials_changed();
  }
}

// static
void supla_user::on_mqtt_settings_changed(int UserID) {
  supla_mqtt_client_suite::globalInstance()->onUserDataChanged(UserID);
}

// static
void supla_user::before_channel_function_change(
    int UserID, int ChannelID, event_source_type eventSourceType) {
  supla_mqtt_client_suite::globalInstance()->beforeChannelFunctionChange(
      UserID, ChannelID);
}

// static
void supla_user::before_device_delete(int UserID, int DeviceID,
                                      event_source_type eventSourceType) {
  supla_mqtt_client_suite::globalInstance()->beforeDeviceDelete(UserID,
                                                                DeviceID);
}

// static
void supla_user::on_device_deleted(int UserID, int DeviceID,
                                   event_source_type eventSourceType) {
  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    supla_http_request_queue::getInstance()->onDeviceDeletedEvent(
        user, 0, eventSourceType);

    supla_mqtt_client_suite::globalInstance()->onDeviceDeleted(UserID,
                                                               DeviceID);
  }
}

// static
void supla_user::on_device_settings_changed(int UserID, int DeviceID,
                                            event_source_type eventSourceType) {
  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    supla_mqtt_client_suite::globalInstance()->onDeviceSettingsChanged(
        UserID, DeviceID);
  }
}

// static
unsigned int supla_user::total_cd_count(bool client) {
  unsigned int result = 0;
  supla_user *user = NULL;

  int a = 0;

  while (NULL != (user = (supla_user *)safe_array_get(user_arr, a))) {
    result += client ? user->client_container->count()
                     : user->device_container->count();
    a++;
  }

  return result;
}

// static
void supla_user::log_metrics(int min_interval_sec) {
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
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

  unsigned int client_count = 0;
  unsigned int client_trash = 0;
  unsigned int device_count = 0;
  unsigned int device_trash = 0;

  int a = 0;
  while (NULL != (user = (supla_user *)safe_array_get(user_arr, a))) {
    serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
    client_count += user->client_container->count();
    serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
    client_trash += user->client_container->trashCount();
    serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
    device_count += user->device_container->count();
    serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
    device_trash += user->device_container->trashCount();
    serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
    a++;
  }

  supla_log(LOG_INFO,
            "METRICS: USER[COUNT:%u] CLIENT[ADD:%u MAX:%u COUNT:%u TRASH:%u] "
            "DEVICE[ADD:%u MAX:%u COUNT:%u TRASH:%u]",
            a, supla_user::client_add_metric, supla_user::client_max_metric,
            client_count, client_trash, supla_user::device_add_metric,
            supla_user::device_max_metric, device_count, device_trash);
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
}

void supla_user::on_channels_added(int DeviceID,
                                   event_source_type eventSourceType) {
  supla_http_request_queue::getInstance()->onChannelsAddedEvent(
      this, DeviceID, eventSourceType);
}

void supla_user::on_device_registered(int DeviceID,
                                      event_source_type eventSourceType) {
  supla_mqtt_client_suite::globalInstance()->onDeviceRegistered(getUserID(),
                                                                DeviceID);
}

bool supla_user::set_device_channel_value(
    event_source_type eventSourceType, int SenderID, int DeviceID,
    int ChannelID, int GroupID, unsigned char EOL,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);
  if (device) {
    // TODO(anyone): Check it out. I think there should be "will change"
    supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
        this, DeviceID, ChannelID, eventSourceType);

    device->get_channels()->set_device_channel_value(SenderID, ChannelID,
                                                     GroupID, EOL, value);
    device->releasePtr();
  }

  return result;
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
                                          bool Extended,
                                          bool SignificantChange) {
  std::list<channel_address> ca_list;
  ca_list.push_back(channel_address(DeviceId, ChannelId));

  supla_device *device = device_container->findByID(DeviceId);
  if (device) {
    std::list<int> master_list =
        device->get_channels()->master_channel(ChannelId);

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

  supla_client *client = NULL;

  for (int a = 0; a < client_container->count(); a++) {
    if (NULL != (client = client_container->get(a))) {
      for (std::list<channel_address>::iterator it = ca_list.begin();
           it != ca_list.end(); it++) {
        client->on_channel_value_changed(it->getDeviceId(), it->getChannelId(),
                                         Extended);
      }
      client->releasePtr();
    }
  }

  for (std::list<channel_address>::iterator it = ca_list.begin();
       it != ca_list.end(); it++) {
    if (SignificantChange && !Extended && DeviceId && ChannelId &&
        eventSourceType != EST_UNKNOWN) {
      supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
          this, it->getDeviceId(), it->getChannelId(), eventSourceType);
    }

    supla_mqtt_client_suite::globalInstance()->onChannelStateChanged(
        getUserID(), it->getDeviceId(), it->getChannelId());
  }
}

void supla_user::on_channel_become_online(int DeviceId, int ChannelId) {
  supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
      this, DeviceId, ChannelId, EST_DEVICE);
  supla_mqtt_client_suite::globalInstance()->onChannelStateChanged(
      getUserID(), DeviceId, ChannelId);
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
      device->get_channels()->get_temp_and_humidity(tarr);
      device->releasePtr();
    }
  }

  database *db = new database();

  if (db->connect() == true) {
    db->load_temperatures_and_humidity(getUserID(), tarr);
  }

  delete db;
}

void supla_user::get_electricity_measurements(void *emarr) {
  int a;
  supla_device *device;

  for (a = 0; a < device_container->count(); a++) {
    if (NULL != (device = device_container->get(a))) {
      device->get_channels()->get_electricity_measurements(emarr);
      device->releasePtr();
    }
  }
}

void supla_user::get_ic_measurements(void *icarr) {
  int a;
  supla_device *device;

  for (a = 0; a < device_container->count(); a++) {
    if (NULL != (device = device_container->get(a))) {
      device->get_channels()->get_ic_measurements(icarr);
      device->releasePtr();
    }
  }
}

void supla_user::get_thermostat_measurements(void *tharr) {
  int a;
  supla_device *device;

  for (a = 0; a < device_container->count(); a++) {
    if (NULL != (device = device_container->get(a))) {
      device->get_channels()->get_thermostat_measurements(tharr);
      device->releasePtr();
    }
  }
}

bool supla_user::device_calcfg_request(int SenderID, int DeviceId,
                                       int ChannelId,
                                       TCS_DeviceCalCfgRequest_B *request) {
  bool result = false;

  if (!request) {
    return false;
  }

  supla_device *device = NULL;

  if (request->Command == SUPLA_CALCFG_CMD_RESET_COUNTERS) {
    device = device_container->findByID(DeviceId);
    if (device) {
      switch (device->get_channels()->get_channel_func(ChannelId)) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH: {
          std::list<int> related =
              device->get_channels()->related_channel(ChannelId);
          if (related.size() == 1) {
            DeviceId = 0;
            ChannelId = related.front();
            device = device_by_channelid(ChannelId);
          }
        } break;
      }
    }
  }

  if (!device && DeviceId) {
    device = device_container->findByID(DeviceId);
  }

  if (device) {
    result = device->get_channels()->calcfg_request(
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

void supla_user::on_device_channel_state_result(int ChannelID,
                                                TDSC_ChannelState *state) {
  if (state == NULL) return;

  supla_client *client = client_container->findByID(state->ReceiverID);

  if (client) {
    client->on_device_channel_state_result(ChannelID, state);
    client->releasePtr();
  }
}

bool supla_user::device_get_channel_state(int SenderID, int DeviceId,
                                          TCSD_ChannelStateRequest *request) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceId);
  if (device) {
    result = device->get_channels()->get_channel_state(SenderID, request);
    device->releasePtr();
  }

  return result;
}

void supla_user::reconnect(event_source_type eventSourceType, bool allDevices,
                           bool allClients) {
  int a;
  if (!allDevices && !allClients) {
    return;
  }

  cgroups->load();  // load == reload

  std::list<cdbase *> cdb;

  if (allDevices) {
    supla_device *device;

    for (a = 0; a < device_container->count(); a++)
      if (NULL != (device = device_container->get(a))) {
        cdb.push_back(device);
      }
  }

  if (allClients) {
    supla_client *client;

    for (a = 0; a < client_container->count(); a++)
      if (NULL != (client = client_container->get(a))) {
        cdb.push_back(client);
      }
  }

  if (cdb.size() > 0) {
    for (std::list<cdbase *>::iterator it = cdb.begin(); it != cdb.end();
         it++) {
      (*it)->terminate();
      (*it)->releasePtr();
    }

    supla_http_request_queue::getInstance()->onUserReconnectEvent(
        this, eventSourceType);
  }
}

void supla_user::reconnect(event_source_type eventSourceType) {
  reconnect(eventSourceType, true, true);
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

bool supla_user::device_reconnect(int DeviceID) {
  bool result = false;

  supla_device *device = device_container->findByID(DeviceID);

  if (device) {
    device->terminate();
    device->releasePtr();
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

// static
bool supla_user::device_reconnect(int UserID, int DeviceID) {
  supla_user *user = find(UserID, true);

  if (user) {
    return user->device_reconnect(DeviceID);
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
                                                    int Type, int Function,
                                                    bool channel_is_hidden) {
  if (!Function || !DeviceId || !ChannelID) return;
  safe_array_lock(complex_value_functions_arr);

  channel_function_t *fnc = NULL;
  if (compex_value_cache_get_function(ChannelID, &fnc).function) {
    if (fnc) {
      fnc->deviceId = DeviceId;
      fnc->channel_type = Type;
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

channel_complex_value supla_user::get_channel_complex_value(int ChannelID) {
  channel_complex_value value;
  memset(&value, 0, sizeof(channel_complex_value));

  supla_device *device = device_container->findByChannelID(ChannelID);
  if (device == NULL) {
    channel_function_t f = compex_value_cache_get_function(ChannelID);
    value.function = f.function;
    value.channel_type = f.channel_type;
    value.hidden_channel = f.channel_is_hidden;
  } else {
    device->get_channels()->get_channel_complex_value(&value, ChannelID);
    if (value.function) {
      compex_value_cache_update_function(device->getID(), ChannelID,
                                         value.channel_type, value.function,
                                         value.hidden_channel);
    }
    device->releasePtr();
  }

  return value;
}

void supla_user::set_channel_function(supla_client *sender,
                                      TCS_SetChannelFunction *func) {
  if (sender == NULL || func == NULL) {
    return;
  }

  TSC_SetChannelFunctionResult result;
  memset(&result, 0, sizeof(TSC_SetChannelFunctionResult));
  result.ChannelID = func->ChannelID;
  result.Func = func->Func;
  result.ResultCode = SUPLA_RESULTCODE_UNKNOWN_ERROR;

  if (!sender->is_superuser_authorized()) {
    result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  } else {
    database *db = new database();

    if (db->connect()) {
      int Type = 0;
      int DeviceID = 0;
      unsigned int FuncList = 0;

      if (!db->get_channel_type_funclist_and_device_id(
              getUserID(), func->ChannelID, &Type, &FuncList, &DeviceID)) {
        result.ResultCode = SUPLA_RESULTCODE_CHANNELNOTFOUND;
      } else if (db->channel_belong_to_group(func->ChannelID)) {
        result.ResultCode = SUPLA_RESULTCODE_DENY_CHANNEL_BELONG_TO_GROUP;
      } else if (db->channel_has_schedule(func->ChannelID)) {
        result.ResultCode = SUPLA_RESULTCODE_DENY_CHANNEL_HAS_SCHEDULE;
      } else if (db->channel_is_associated_with_scene(func->ChannelID)) {
        result.ResultCode =
            SUPLA_RESULTCODE_DENY_CHANNEL_IS_ASSOCIETED_WITH_SCENE;
      } else if (db->channel_is_associated_with_action_trigger(
                     getUserID(), func->ChannelID)) {
        result.ResultCode =
            SUPLA_RESULTCODE_DENY_CHANNEL_IS_ASSOCIETED_WITH_ACTION_TRIGGER;
      } else {
        if (Type != SUPLA_CHANNELTYPE_BRIDGE ||
            (func->Func != SUPLA_CHANNELFNC_NONE &&
             !supla_device::funclist_contains_function(FuncList, func->Func))) {
          result.ResultCode = SUPLA_RESULTCODE_NOT_ALLOWED;
        } else {
          before_channel_function_change(getUserID(), func->ChannelID,
                                         EST_CLIENT);
          if (db->set_channel_function(getUserID(), func->ChannelID,
                                       func->Func)) {
            result.ResultCode = SUPLA_RESULTCODE_TRUE;
            supla_mqtt_client_suite::globalInstance()->onDeviceSettingsChanged(
                getUserID(), DeviceID);
          } else {
            result.ResultCode = SUPLA_RESULTCODE_UNKNOWN_ERROR;
          }
        }
      }
    } else {
      result.ResultCode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
    }
    delete db;
  }

  if (result.ResultCode == SUPLA_RESULTCODE_TRUE) {
    supla_device *device = device_container->findByChannelID(func->ChannelID);
    if (device != NULL) {
      device->get_channels()->set_channel_function(func->ChannelID, func->Func);
      device->releasePtr();
    }

    supla_client *client = NULL;

    for (int a = 0; a < client_container->count(); a++)
      if (NULL != (client = client_container->get(a))) {
        client->set_channel_function(func->ChannelID, func->Func);
        client->releasePtr();
      }
  }

  sender->set_channel_function_result(&result);
}

void supla_user::set_caption(supla_client *sender, TCS_SetCaption *caption,
                             bool channel) {
  if (sender == NULL || caption == NULL) {
    return;
  }

  TSC_SetCaptionResult result;
  memset(&result, 0, sizeof(TSC_SetCaptionResult));
  result.ID = caption->ID;
  memcpy(result.Caption, caption->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE);
  result.CaptionSize = caption->CaptionSize;
  if (result.CaptionSize > SUPLA_CHANNEL_CAPTION_MAXSIZE) {
    result.CaptionSize = SUPLA_CHANNEL_CAPTION_MAXSIZE;
  }
  result.ResultCode = SUPLA_RESULTCODE_UNKNOWN_ERROR;

  if (!sender->is_superuser_authorized()) {
    result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  } else {
    database *db = new database();

    if (db->connect()) {
      if (db->set_caption(getUserID(), caption->ID, caption->Caption,
                          channel)) {
        result.ResultCode = SUPLA_RESULTCODE_TRUE;
      } else {
        result.ResultCode = SUPLA_RESULTCODE_UNKNOWN_ERROR;
      }
    } else {
      result.ResultCode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
    }
    delete db;
  }

  if (result.ResultCode == SUPLA_RESULTCODE_TRUE) {
    supla_client *client = NULL;

    for (int a = 0; a < client_container->count(); a++)
      if (NULL != (client = client_container->get(a))) {
        if (channel) {
          client->set_channel_caption(caption->ID, caption->Caption);
        } else {
          client->set_location_caption(caption->ID, caption->Caption);
        }

        client->releasePtr();
      }
  }

  sender->set_caption_result(&result, channel);
}

supla_amazon_alexa_credentials *supla_user::amazonAlexaCredentials(void) {
  return amazon_alexa_credentials;
}

supla_google_home_credentials *supla_user::googleHomeCredentials(void) {
  return google_home_credentials;
}

supla_state_webhook_credentials *supla_user::stateWebhookCredentials(void) {
  return state_webhook_credentials;
}

supla_user_channelgroups *supla_user::get_channel_groups(void) {
  return cgroups;
}
