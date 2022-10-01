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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <userclients.h>
#include <userdevices.h>

#include <list>

#include "amazon/alexacredentials.h"
#include "client.h"
#include "db/database.h"
#include "device.h"
#include "google/googlehomecredentials.h"
#include "http/httprequestqueue.h"
#include "lck.h"
#include "log.h"
#include "mqtt/mqtt_client_suite.h"
#include "safearray.h"
#include "scene/scene_asynctask.h"
#include "serverstatus.h"
#include "userchannelgroups.h"

void *supla_user::user_arr = NULL;
unsigned int supla_user::client_add_metric = 0;
unsigned int supla_user::client_max_metric = 0;
unsigned int supla_user::device_add_metric = 0;
unsigned int supla_user::device_max_metric = 0;
struct timeval supla_user::metric_tv = (struct timeval){0};

using std::function;
using std::list;
using std::shared_ptr;
using std::vector;

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

  this->devices = new supla_user_devices();
  this->clients = new supla_user_clients();
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

  delete devices;
  delete clients;
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
supla_user *supla_user::get_user(int user_id) { return find(user_id, false); }

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
vector<supla_user *> supla_user::get_all_users(void) {
  vector<supla_user *> result;
  safe_array_lock(supla_user::user_arr);
  int a = 0;

  supla_user *user = NULL;
  while (NULL != (user = (supla_user *)safe_array_get(user_arr, a))) {
    result.push_back(user);
    a++;
  }
  safe_array_unlock(supla_user::user_arr);
  return result;
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

supla_user_devices *supla_user::get_devices() { return devices; }

supla_user_clients *supla_user::get_clients() { return clients; }

supla_user *supla_user::add_device(shared_ptr<supla_device> device,
                                   int user_id) {
  assert(device != nullptr);
  assert(device->get_id() != 0);

  supla_user *user = find(user_id, true);

  user->compex_value_cache_clean(device->get_id());

  if (user->devices->add(device)) {
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

supla_user *supla_user::add_client(shared_ptr<supla_client> client,
                                   int user_id) {
  assert(client != nullptr);
  assert(client->get_id() != 0);

  supla_user *user = find(user_id, true);

  if (user->clients->add(client)) {
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

// static
std::shared_ptr<supla_device> supla_user::get_device(int user_id, int device_id,
                                                     int channel_id) {
  supla_user *user = find(user_id, false);
  if (user) {
    return user->get_devices()->get(device_id, channel_id);
  }
  return nullptr;
}

// static
std::shared_ptr<supla_client> supla_user::get_client(int user_id,
                                                     int client_id) {
  supla_user *user = find(user_id, false);
  if (user) {
    return user->get_clients()->get(client_id);
  }
  return nullptr;
}

bool supla_user::get_channel_value(
    int device_id, int channel_id, char value[SUPLA_CHANNELVALUE_SIZE],
    char sub_value[SUPLA_CHANNELVALUE_SIZE], char *sub_value_type,
    TSuplaChannelExtendedValue *ev, int *function, char *online,
    unsigned _supla_int_t *validity_time_sec, bool for_client) {
  bool result = false;
  memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
  memset(sub_value, 0, SUPLA_CHANNELVALUE_SIZE);
  if (online) {
    *online = 0;
  }

  shared_ptr<supla_device> related_device;
  shared_ptr<supla_device> device = devices->get(device_id);
  if (device != nullptr) {
    result = device->get_channels()->get_channel_value(
        channel_id, value, online, validity_time_sec, ev, function, for_client);

    if (result) {
      list<int> related_list =
          device->get_channels()->related_channel(channel_id);

      auto it = related_list.begin();
      bool sub_value_exists = false;
      char sub_channel_online = 0;

      if (related_list.size() == 1 && *it > 0) {
        related_device = devices->get(0, *it);
        if (related_device != nullptr) {
          if (related_device->get_channels()->get_channel_value(
                  *it, sub_value, &sub_channel_online, nullptr, nullptr,
                  nullptr, for_client) &&
              sub_channel_online) {
            sub_value_exists = true;
          } else {
            memset(sub_value, 0, SUPLA_CHANNELVALUE_SIZE);
          }

          related_device = nullptr;
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
            related_device = devices->get(0, *it);
            if (related_device != nullptr) {
              sub_channel_online = false;
              if (related_device->get_channels()->get_channel_value(
                      *it, _sub_value, &sub_channel_online, nullptr, nullptr,
                      nullptr, for_client) &&
                  sub_channel_online) {
                sub_value[n] = _sub_value[0];
                sub_value_exists = true;
              }

              related_device = nullptr;
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
          related_device = devices->get(0, *it);
          if (related_device != nullptr) {
            int rel_channel_func =
                related_device->get_channels()->get_channel_func(*it);

            related_device = nullptr;

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
void supla_user::before_channel_function_change(int UserID, int ChannelID,
                                                const supla_caller &caller) {
  supla_mqtt_client_suite::globalInstance()->beforeChannelFunctionChange(
      UserID, ChannelID);
}

// static
void supla_user::before_device_delete(int UserID, int DeviceID,
                                      const supla_caller &caller) {
  supla_mqtt_client_suite::globalInstance()->beforeDeviceDelete(UserID,
                                                                DeviceID);
}

// static
void supla_user::on_device_deleted(int UserID, int DeviceID,
                                   const supla_caller &caller) {
  supla_user *user = supla_user::find(UserID, false);

  if (user) {
    user->get_devices()->terminate(DeviceID);

    supla_http_request_queue::getInstance()->onDeviceDeletedEvent(user, 0,
                                                                  caller);

    supla_mqtt_client_suite::globalInstance()->onDeviceDeleted(UserID,
                                                               DeviceID);
  }
}

// static
void supla_user::on_device_settings_changed(int UserID, int DeviceID,
                                            const supla_caller &caller) {
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
    result += client ? user->clients->count() : user->devices->count();
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

  unsigned int client_count = 0;
  unsigned int client_trash = 0;
  unsigned int device_count = 0;
  unsigned int device_trash = 0;

  vector<supla_user *> users = supla_user::get_all_users();

  int n = 0;
  for (auto uit = users.cbegin(); uit != users.cend(); ++uit) {
    serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
    client_count += (*uit)->clients->count();
    serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
    device_count += (*uit)->devices->count();
    n++;
  }

  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);

  supla_log(LOG_INFO,
            "METRICS: USER[COUNT:%u] CLIENT[ADD:%u MAX:%u COUNT:%u TRASH:%u] "
            "DEVICE[ADD:%u MAX:%u COUNT:%u TRASH:%u]",
            n, supla_user::client_add_metric, supla_user::client_max_metric,
            client_count, client_trash, supla_user::device_add_metric,
            supla_user::device_max_metric, device_count, device_trash);
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
}

void supla_user::on_channels_added(int DeviceID, const supla_caller &caller) {
  supla_http_request_queue::getInstance()->onChannelsAddedEvent(this, DeviceID,
                                                                caller);
}

void supla_user::on_device_registered(int DeviceID,
                                      const supla_caller &caller) {
  supla_mqtt_client_suite::globalInstance()->onDeviceRegistered(getUserID(),
                                                                DeviceID);
}

bool supla_user::set_device_channel_value(
    const supla_caller &caller, int device_id, int channel_id, int group_id,
    unsigned char eol, const char value[SUPLA_CHANNELVALUE_SIZE]) {
  bool result = false;

  shared_ptr<supla_device> device = devices->get(device_id);
  if (device != nullptr) {
    // TODO(anyone): Check it out. I think there should be "will change"
    supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
        this, device_id, channel_id, caller);

    device->get_channels()->set_device_channel_value(caller, channel_id,
                                                     group_id, eol, value);
  }

  return result;
}

void supla_user::on_channel_value_changed(const supla_caller &caller,
                                          int device_id, int channel_id,
                                          bool extended,
                                          bool significant_change) {
  list<channel_address> ca_list;
  ca_list.push_back(channel_address(device_id, channel_id));

  shared_ptr<supla_device> device = devices->get(device_id);
  if (device != nullptr) {
    list<int> master_list = device->get_channels()->master_channel(channel_id);

    device = nullptr;

    for (auto it = master_list.begin(); it != master_list.end(); it++) {
      device = devices->get(0, *it);

      if (device != nullptr) {
        ca_list.push_back(channel_address(device->get_id(), *it));
        device = nullptr;
      }
    }
  }

  clients->on_channel_value_changed(ca_list, extended);

  for (auto it = ca_list.begin(); it != ca_list.end(); it++) {
    if (significant_change && !extended && device_id && channel_id &&
        caller != ctUnknown) {
      supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
          this, it->getDeviceId(), it->getChannelId(), caller);
    }

    supla_mqtt_client_suite::globalInstance()->onChannelStateChanged(
        getUserID(), it->getDeviceId(), it->getChannelId());
  }
}

void supla_user::on_channel_become_online(int DeviceId, int ChannelId) {
  supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
      this, DeviceId, ChannelId, supla_caller(ctDevice, DeviceId));
  supla_mqtt_client_suite::globalInstance()->onChannelStateChanged(
      getUserID(), DeviceId, ChannelId);
}

bool supla_user::device_calcfg_request(const supla_caller &caller,
                                       int device_id, int channel_id,
                                       TCS_DeviceCalCfgRequest_B *request) {
  bool result = false;

  if (!request) {
    return false;
  }

  shared_ptr<supla_device> device = nullptr;

  if (request->Command == SUPLA_CALCFG_CMD_RESET_COUNTERS) {
    device = devices->get(device_id);
    if (device != nullptr) {
      switch (device->get_channels()->get_channel_func(channel_id)) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH: {
          list<int> related =
              device->get_channels()->related_channel(channel_id);
          if (related.size() == 1) {
            device_id = 0;
            channel_id = related.front();
            device = devices->get(0, channel_id);
          }
        } break;
      }
    }
  }

  if (!device && device_id) {
    device = devices->get(device_id);
  }

  if (device) {
    bool superUserAuthorized = false;
    if (caller == ctClient && caller.get_id() > 0) {
      shared_ptr<supla_client> client = clients->get(caller.get_id());
      superUserAuthorized =
          client != nullptr && client->is_superuser_authorized();
    }
    result = device->get_channels()->calcfg_request(
        caller, channel_id, superUserAuthorized, request);
  }

  return result;
}

void supla_user::reconnect(const supla_caller &caller, bool all_devices,
                           bool all_clients) {
  if (!all_devices && !all_clients) {
    return;
  }

  cgroups->load();  // load == reload

  bool any_terminated = false;

  if (all_devices && devices->reconnect_all()) {
    any_terminated = true;
  }

  if (all_clients && clients->reconnect_all()) {
    any_terminated = true;
  }

  if (any_terminated) {
    supla_http_request_queue::getInstance()->onUserReconnectEvent(this, caller);
  }
}

void supla_user::reconnect(const supla_caller &caller) {
  reconnect(caller, true, true);
}

// static
bool supla_user::reconnect(int UserID, const supla_caller &caller) {
  supla_user *user = find(UserID, true);

  if (user) {
    user->reconnect(caller);
    // cppcheck-suppress memleak
    return true;
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

channel_complex_value supla_user::get_channel_complex_value(int channel_id) {
  channel_complex_value value = {};

  shared_ptr<supla_device> device = devices->get(0, channel_id);
  if (device == nullptr) {
    channel_function_t f = compex_value_cache_get_function(channel_id);
    value.function = f.function;
    value.channel_type = f.channel_type;
    value.hidden_channel = f.channel_is_hidden;
  } else {
    device->get_channels()->get_channel_complex_value(&value, channel_id);
    if (value.function) {
      compex_value_cache_update_function(device->get_id(), channel_id,
                                         value.channel_type, value.function,
                                         value.hidden_channel);
    }
  }

  return value;
}

void supla_user::set_channel_function(std::shared_ptr<supla_client> sender,
                                      TCS_SetChannelFunction *func) {
  if (sender == nullptr || func == NULL) {
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
          before_channel_function_change(
              getUserID(), func->ChannelID,
              supla_caller(ctClient, sender->get_id()));
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
    shared_ptr<supla_device> device = devices->get(0, func->ChannelID);
    if (device != nullptr) {
      device->get_channels()->set_channel_function(func->ChannelID, func->Func);
    }

    clients->set_channel_function(func->ChannelID, func->Func);
  }

  sender->set_channel_function_result(&result);
}

void supla_user::set_caption(std::shared_ptr<supla_client> sender,
                             TCS_SetCaption *caption, bool channel) {
  if (sender == nullptr || caption == NULL) {
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
    if (channel) {
      clients->set_channel_caption(caption->ID, caption->Caption);
    } else {
      clients->set_location_caption(caption->ID, caption->Caption);
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

// static
void supla_user::on_scene_changed(const supla_caller &caller, int user_id,
                                  int scene_id) {
  supla_user *user = find(user_id, false);
  if (user) {
    user->reconnect(caller, false, true);
    supla_scene_asynctask::interrupt(supla_scene_asynctask::get_queue(),
                                     user_id, scene_id);
  }
}
