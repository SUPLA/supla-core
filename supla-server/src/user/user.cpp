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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <list>
#include "amazon/alexa.h"

#include "client.h"
#include "database.h"
#include "device.h"
#include "http/httprequestqueue.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "user.h"
#include "userchannelgroups.h"

void *supla_user::user_arr = NULL;

// static
char supla_user::find_user_byid(void *ptr, void *UserID) {
  return ((supla_user *)ptr)->getUserID() == *(int *)UserID ? 1 : 0;
}

// static
char supla_user::find_device_byid(void *ptr, void *ID) {
  return ((supla_device *)ptr)->getID() == *(int *)ID ? 1 : 0;
}

// static
char supla_user::find_device_by_channelid(void *ptr, void *ID) {
  return ((supla_device *)ptr)->channel_exists(*(int *)ID) ? 1 : 0;
}

// static
char supla_user::find_device_byguid(void *ptr, void *GUID) {
  return ((supla_device *)ptr)->cmpGUID((char *)GUID) ? 1 : 0;
}

// static
char supla_user::find_client_byid(void *ptr, void *ID) {
  return ((supla_client *)ptr)->getID() == *(int *)ID ? 1 : 0;
}

// static
char supla_user::find_client_byguid(void *ptr, void *GUID) {
  return ((supla_client *)ptr)->cmpGUID((char *)GUID) ? 1 : 0;
}

supla_user::supla_user(int UserID) {
  this->UserID = UserID;
  this->device_arr = safe_array_init();
  this->client_arr = safe_array_init();
  this->cgroups = new supla_user_channelgroups(this);
  this->amazon_alexa = new supla_amazon_alexa(this);
  this->amazon_alexa->load();
  this->connections_allowed = true;
  this->short_unique_id = NULL;
  this->long_unique_id = NULL;
  this->lck = lck_init();

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
  safe_array_free(device_arr);
  safe_array_free(client_arr);
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

bool supla_user::getClientName(int ClientID, char *buffer, int size) {
  if (size < 1) return false;

  buffer[0] = 0;

  safe_array_lock(client_arr);

  supla_client *client = find_client(ClientID);

  if (client) client->getName(buffer, size);

  safe_array_unlock(client_arr);

  return client != NULL;
}

bool supla_user::isSuperUserAuthorized(int ClientID) {
  bool result = false;
  safe_array_lock(client_arr);

  supla_client *client = find_client(ClientID);

  if (client) result = client->is_superuser_authorized();

  safe_array_unlock(client_arr);
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

  safe_array_lock(user->device_arr);

  if (safe_array_find(user->device_arr, device) == -1) {
    char GUID[SUPLA_GUID_SIZE];
    memset(GUID, 0, SUPLA_GUID_SIZE);
    device->getGUID(GUID);

    supla_device *_device = (supla_device *)safe_array_findcnd(
        user->device_arr, find_device_byguid, GUID);
    if (_device != NULL) {
      _device->terminate();
      usleep(1000000);
    }

    safe_array_add(user->device_arr, device);
  }

  safe_array_unlock(user->device_arr);

  return user;
}

supla_user *supla_user::add_client(supla_client *client, int UserID) {
  supla_user *user = find(UserID, true);

  safe_array_lock(user->client_arr);

  if (safe_array_find(user->client_arr, client) == -1) {
    char GUID[SUPLA_GUID_SIZE];
    memset(GUID, 0, SUPLA_GUID_SIZE);
    client->getGUID(GUID);

    supla_client *_client = (supla_client *)safe_array_findcnd(
        user->client_arr, find_client_byguid, GUID);
    if (_client != NULL) {
      _client->terminate();
      usleep(1000000);
    }

    safe_array_add(user->client_arr, client);
  }

  safe_array_unlock(user->client_arr);

  return user;
}

void supla_user::remove_device(supla_device *device) {
  safe_array_remove(device_arr, device);
}

void supla_user::remove_client(supla_client *client) {
  safe_array_remove(client_arr, client);
}

supla_device *supla_user::find_device(int DeviceID) {
  return (supla_device *)safe_array_findcnd(device_arr, find_device_byid,
                                            &DeviceID);
}

supla_device *supla_user::find_device_by_channelid(int ChannelID) {
  if (ChannelID == 0) return NULL;
  return (supla_device *)safe_array_findcnd(
      device_arr, find_device_by_channelid, &ChannelID);
}

supla_client *supla_user::find_client(int ClientID) {
  return (supla_client *)safe_array_findcnd(client_arr, find_client_byid,
                                            &ClientID);
}

bool supla_user::is_client_online(int ClientID) {
  return find_client(ClientID) != NULL;
}

bool supla_user::is_device_online(int DeviceID) {
  return find_device(DeviceID) != NULL;
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

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceID);
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
  }

  safe_array_unlock(device_arr);

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

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceID);

  if (device != NULL) {
    result = device->get_channel_char_value(ChannelID, Value) == 1;
  }

  safe_array_unlock(device_arr);

  return result;
}

bool supla_user::get_channel_rgbw_value(int DeviceID, int ChannelID, int *color,
                                        char *color_brightness,
                                        char *brightness, char *on_off) {
  bool result = false;

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceID);
  if (device != NULL) {
    result = device->get_channel_rgbw_value(ChannelID, color, color_brightness,
                                            brightness, on_off) == 1;
  }

  safe_array_unlock(device_arr);

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

supla_device *supla_user::device_by_channel_id(supla_device *suspect,
                                               int ChannelID) {
  if (ChannelID == 0) return NULL;
  if (suspect != NULL && suspect->channel_exists(ChannelID)) return suspect;

  for (int a = 0; a < safe_array_count(device_arr); a++)
    if (NULL != (suspect = (supla_device *)safe_array_get(device_arr, a)) &&
        suspect->channel_exists(ChannelID))
      return suspect;

  return NULL;
}

bool supla_user::get_channel_value(int DeviceID, int ChannelID,
                                   TSuplaChannelValue *value, char *online) {
  bool result = false;
  memset(value, 0, sizeof(TSuplaChannelValue));
  *online = 0;

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceID);
  if (device) {
    *online = 1;
    result = device->get_channel_value(ChannelID, value->value);

    if (result) {
      std::list<int> slave_list = device->slave_channel(ChannelID);

      std::list<int>::iterator it = slave_list.begin();

      if (slave_list.size() == 1 && *it > 0) {
        device = device_by_channel_id(device, *it);
        if (device) {
          device->get_channel_value(*it, value->sub_value);
        }
      } else if (slave_list.size() > 1) {
        // For more than one sub-channel
        // copy only first byte of sub_value
        // At this version only SENSORNO/SENSORNC are supported.
        // See supla_device_channel::slave_channel
        char sub_value[SUPLA_CHANNELVALUE_SIZE];
        int n = 0;
        do {
          if (*it > 0) {
            device = device_by_channel_id(device, *it);
            if (device) {
              device->get_channel_value(*it, sub_value);
              value->sub_value[n] = sub_value[0];
            }
          }
          n++;
          it++;
        } while (it != slave_list.end() && n < SUPLA_CHANNELVALUE_SIZE);
      }
    }
  }

  safe_array_unlock(device_arr);

  return result;
}

bool supla_user::get_channel_extendedvalue(int DeviceID, int ChannelID,
                                           TSuplaChannelExtendedValue *value) {
  bool result = false;
  memset(value, 0, sizeof(TSuplaChannelExtendedValue));
  safe_array_lock(device_arr);
  supla_device *device = find_device(DeviceID);
  if (device) {
    result = device->get_channel_extendedvalue(ChannelID, value) > 0;
  }

  safe_array_unlock(device_arr);

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
      supla_http_request_queue::getInstance()->onChannelChangeEvent(
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
      supla_http_request_queue::getInstance()->onChannelChangeEvent(
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
  }

  safe_array_unlock(supla_user::user_arr);
}

// static
void supla_user::on_device_deleted(int UserID) {}

bool supla_user::set_device_channel_value(
    event_source_type eventSourceType, int SenderID, int DeviceID,
    int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]) {
  bool result = false;

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceID);
  if (device) {
    supla_http_request_queue::getInstance()->onChannelChangeEvent(
        this, DeviceID, ChannelID, eventSourceType);

    device->set_device_channel_value(SenderID, ChannelID, value);
  }

  safe_array_unlock(device_arr);

  return result;
}

bool supla_user::set_device_channel_char_value(int SenderID, int DeviceID,
                                               int ChannelID,
                                               const char value) {
  bool result = false;

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceID);
  if (device)
    result = device->set_device_channel_char_value(SenderID, ChannelID, value);

  safe_array_unlock(device_arr);

  return result;
}

bool supla_user::set_device_channel_rgbw_value(int SenderID, int DeviceID,
                                               int ChannelID, int color,
                                               char color_brightness,
                                               char brightness, char on_off) {
  bool result = false;

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceID);
  if (device)
    result = device->set_device_channel_rgbw_value(
        SenderID, ChannelID, color, color_brightness, brightness, on_off);

  safe_array_unlock(device_arr);

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
  safe_array_lock(client_arr);
  {
    supla_client *client;

    for (int a = 0; a < safe_array_count(client_arr); a++)
      if (NULL != (client = (supla_client *)safe_array_get(client_arr, a))) {
        client->update_device_channels(LocationID, DeviceID);
      }
  }
  safe_array_unlock(client_arr);
}

void supla_user::on_channel_value_changed(event_source_type eventSourceType,
                                          int DeviceId, int ChannelId,
                                          bool Extended) {
  if (!Extended && DeviceId && ChannelId && eventSourceType != EST_UNKNOWN) {
    supla_http_request_queue::getInstance()->onChannelChangeEvent(
        this, DeviceId, ChannelId, eventSourceType);
  }

  std::list<channel_address> ca_list;

  if (Extended) {
    safe_array_lock(client_arr);
    {
      supla_client *client;

      for (int a = 0; a < safe_array_count(client_arr); a++)
        if (NULL != (client = (supla_client *)safe_array_get(client_arr, a))) {
          client->on_channel_value_changed(DeviceId, ChannelId, true);
        }
    }
    safe_array_unlock(client_arr);
    return;
  }

  safe_array_lock(device_arr);
  {
    supla_device *device = find_device(DeviceId);
    if (device) {
      std::list<int> master_list = device->master_channel(ChannelId);
      for (std::list<int>::iterator it = master_list.begin();
           it != master_list.end(); it++) {
        device = find_device_by_channelid(*it);

        if (device) {
          ca_list.push_back(channel_address(device->getID(), *it));
        }
      }
    }
  }
  safe_array_unlock(device_arr);

  if (ca_list.empty()) {
    ca_list.push_back(channel_address(DeviceId, ChannelId));
  }

  safe_array_lock(client_arr);
  {
    supla_client *client;

    for (int a = 0; a < safe_array_count(client_arr); a++)
      if (NULL != (client = (supla_client *)safe_array_get(client_arr, a))) {
        for (std::list<channel_address>::iterator it = ca_list.begin();
             it != ca_list.end(); it++) {
          client->on_channel_value_changed(it->getDeviceId(),
                                           it->getChannelId());
        }
      }
  }
  safe_array_unlock(client_arr);
}

void supla_user::on_channel_become_online(int DeviceId, int ChannelId) {
  supla_http_request_queue::getInstance()->onChannelChangeEvent(
      this, DeviceId, ChannelId, EST_DEVICE);
}

void supla_user::call_event(TSC_SuplaEvent *event) {
  safe_array_lock(client_arr);
  {
    supla_client *client;

    for (int a = 0; a < safe_array_count(client_arr); a++)
      if (NULL != (client = (supla_client *)safe_array_get(client_arr, a))) {
        client->call_event(event);
      }
  }
  safe_array_unlock(client_arr);
}

void supla_user::get_temp_and_humidity(void *tarr) {
  int a;

  safe_array_lock(device_arr);

  for (a = 0; a < safe_array_count(device_arr); a++) {
    ((supla_device *)safe_array_get(device_arr, a))
        ->get_temp_and_humidity(tarr);
  }

  safe_array_unlock(device_arr);
}

void supla_user::get_electricity_measurement(void *emarr) {
  int a;

  safe_array_lock(device_arr);

  for (a = 0; a < safe_array_count(device_arr); a++) {
    ((supla_device *)safe_array_get(device_arr, a))
        ->get_electricity_measurement(emarr);
  }

  safe_array_unlock(device_arr);
}

void supla_user::get_ic_measurement(void *icarr) {
  int a;

  safe_array_lock(device_arr);

  for (a = 0; a < safe_array_count(device_arr); a++) {
    ((supla_device *)safe_array_get(device_arr, a))
        ->get_ic_measurement(icarr);
  }

  safe_array_unlock(device_arr);
}

bool supla_user::device_calcfg_request(int SenderID, int DeviceId,
                                       TCS_DeviceCalCfgRequest *request) {
  bool result = false;

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceId);
  if (device)
    result = device->calcfg_request(
        SenderID, SenderID > 0 ? isSuperUserAuthorized(SenderID) : false,
        request);

  safe_array_unlock(device_arr);

  return result;
}

void supla_user::on_device_calcfg_result(int ChannelID,
                                         TDS_DeviceCalCfgResult *result) {
  if (result == NULL) return;
  safe_array_lock(client_arr);

  supla_client *client = find_client(result->SenderID);

  if (client) {
    client->on_device_calcfg_result(ChannelID, result);
  }

  safe_array_unlock(client_arr);
}

void supla_user::reconnect() {
  int a;

  cgroups->load();  // load == reload

  safe_array_lock(device_arr);
  {
    supla_device *device;

    for (a = 0; a < safe_array_count(device_arr); a++)
      if (NULL != (device = (supla_device *)safe_array_get(device_arr, a))) {
        device->terminate();
      }
  }
  safe_array_unlock(device_arr);

  safe_array_lock(client_arr);
  {
    supla_client *client;

    for (a = 0; a < safe_array_count(client_arr); a++)
      if (NULL != (client = (supla_client *)safe_array_get(client_arr, a))) {
        client->terminate();
      }
  }
  safe_array_unlock(client_arr);
}

bool supla_user::client_reconnect(int ClientID) {
  bool result = false;

  safe_array_lock(client_arr);

  supla_client *client = find_client(ClientID);

  if (client) {
    client->terminate();
    result = true;
  }

  safe_array_unlock(client_arr);

  return result;
}

// static
bool supla_user::reconnect(int UserID) {
  supla_user *user = find(UserID, true);

  if (user) {
    user->reconnect();
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

channel_complex_value supla_user::get_channel_complex_value(int DeviceId,
                                                            int ChannelID) {
  channel_complex_value value;
  memset(&value, 0, sizeof(channel_complex_value));

  safe_array_lock(device_arr);

  supla_device *device = find_device(DeviceId);
  if (device != NULL) {
    device->get_channel_complex_value(&value, ChannelID);
  }

  safe_array_unlock(device_arr);
  return value;
}

supla_amazon_alexa *supla_user::amazonAlexa(void) { return amazon_alexa; }
