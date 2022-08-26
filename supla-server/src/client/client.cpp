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

#include "client.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "clientlocation.h"
#include "database.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "scene/scene_asynctask.h"
#include "srpc/srpc.h"
#include "user.h"
#include "user/userchannelgroups.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;

supla_client::supla_client(supla_connection *connection)
    : supla_abstract_connection_object(connection) {
  this->locations = new supla_client_locations();
  this->channels = new supla_client_channels(this);
  this->cgroups = new supla_client_channelgroups(this);
  this->scene_remote_updater =
      new supla_client_scene_remote_updater(connection->get_srpc_adapter());
  this->scenes = new supla_client_scenes(scene_remote_updater, &scene_dao,
                                         supla_scene_asynctask::get_queue());
  this->name[0] = 0;
  this->superuser_authorized = false;
  this->access_id = 0;
}

supla_client::~supla_client() {
  delete cgroups;
  delete channels;
  delete locations;
  delete scenes;
  delete scene_remote_updater;
}

supla_abstract_srpc_call_handler_collection *
supla_client::get_srpc_call_handler_collection(void) {
  return NULL;
}

shared_ptr<supla_client> supla_client::get_shared_ptr(void) {
  return dynamic_pointer_cast<supla_client>(
      supla_abstract_connection_object::get_shared_ptr());
}

supla_client_channels *supla_client::get_channels(void) { return channels; }

supla_client_channelgroups *supla_client::get_cgroups(void) { return cgroups; }

void supla_client::setName(const char *name) {
  lck_lock(lck);
  snprintf(this->name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", name);
  this->name[SUPLA_CLIENT_NAME_MAXSIZE - 1] = 0;
  lck_unlock(lck);
}

int supla_client::getName(char *buffer, int size) {
  if (size < 1) return 0;

  buffer[0] = 0;

  lck_lock(lck);
  snprintf(buffer, size, "%s", this->name);
  lck_unlock(lck);

  buffer[size - 1] = 0;
  return strnlen(buffer, size - 1);
}

void supla_client::setAccessID(int AccessID) { access_id = AccessID; }

int supla_client::getAccessID(void) { return access_id; }

void supla_client::revoke_superuser_authorization(void) {
  lck_lock(lck);
  superuser_authorized = false;
  lck_unlock(lck);
}

bool supla_client::is_superuser_authorized(void) {
  bool result = false;
  lck_lock(lck);
  result = superuser_authorized;
  lck_unlock(lck);

  return result;
}

bool supla_client::db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                                   const char Email[SUPLA_EMAIL_MAXSIZE],
                                   const char AuthKey[SUPLA_AUTHKEY_SIZE],
                                   int *UserID, database *db) {
  return db->client_authkey_auth(GUID, Email, AuthKey, UserID);
}

char supla_client::register_client(TCS_SuplaRegisterClient_B *register_client_b,
                                   TCS_SuplaRegisterClient_D *register_client_d,
                                   unsigned char proto_version) {
  int resultcode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
  char result = 0;

  char *GUID = NULL;
  char *AuthKey = NULL;
  char *Name = NULL;
  char *SoftVer = NULL;
  _supla_int_t AccessID = 0;

  if (register_client_b != NULL) {
    GUID = register_client_b->GUID;
    Name = register_client_b->Name;
    AccessID = register_client_b->AccessID;
    SoftVer = register_client_b->SoftVer;
  } else {
    GUID = register_client_d->GUID;
    Name = register_client_d->Name;
    AuthKey = register_client_d->AuthKey;
    SoftVer = register_client_d->SoftVer;
  }

  if (!set_guid(GUID)) {
    resultcode = SUPLA_RESULTCODE_GUID_ERROR;

  } else if (register_client_d != NULL &&
             !set_authkey(register_client_d->AuthKey)) {
    resultcode = SUPLA_RESULTCODE_AUTHKEY_ERROR;

  } else if (register_client_b == NULL && register_client_d == NULL) {
    resultcode = SUPLA_RESULTCODE_UNSUPORTED;

  } else {
    database *db = new database();

    if (db->connect() == true) {
      int UserID = 0;
      bool pwd_is_set = false;
      bool accessid_enabled = false;
      bool accessid_active = false;

      if (register_client_b != NULL &&
          false == db->accessid_auth(AccessID, register_client_b->AccessIDpwd,
                                     &UserID, &accessid_enabled,
                                     &accessid_active)) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (register_client_d != NULL &&
                 false == authkey_auth(GUID, register_client_d->Email,
                                       register_client_d->AuthKey, &UserID,
                                       db)) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (UserID == 0) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else {
        _supla_int_t _AccessID = AccessID;

        bool client_enabled = true;
        bool do_update = true;
        bool _accessid_enabled = false;
        bool _accessid_active = false;

        db->start_transaction();

        int ClientID =
            db->get_client(db->get_client_id(UserID, GUID), &client_enabled,
                           &_AccessID, &_accessid_enabled, &_accessid_active);

        if (_accessid_enabled) accessid_enabled = true;
        if (_accessid_active) accessid_active = true;

        pwd_is_set =
            register_client_d != NULL &&
            strnlen(register_client_d->Password, SUPLA_PASSWORD_MAXSIZE) > 0;

        if (ClientID == 0) {
          do_update = false;

          if (pwd_is_set) {
            superuser_authorize(UserID, register_client_d->Email,
                                register_client_d->Password, NULL);
          }

          if (false == db->get_client_reg_enabled(UserID) &&
              false == is_superuser_authorized()) {
            db->rollback();
            resultcode = pwd_is_set ? SUPLA_RESULTCODE_BAD_CREDENTIALS
                                    : SUPLA_RESULTCODE_REGISTRATION_DISABLED;

          } else if (db->get_client_limit_left(UserID) <= 0) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED;

          } else {
            if (AccessID == 0 && register_client_d != NULL &&
                (db->get_client_count(UserID) == 0 ||
                 is_superuser_authorized())) {
              AccessID = db->get_access_id(UserID, true, true);

              if (AccessID > 0) {
                accessid_enabled = true;
                accessid_active = true;
              } else {
                accessid_enabled = false;
                AccessID = db->get_access_id(UserID, false, true);
                if (AccessID > 0) {
                  accessid_active = true;
                } else {
                  accessid_active = false;
                  AccessID = db->get_access_id(UserID, false, false);
                }
              }
            }

            ClientID = db->add_client(AccessID, GUID, AuthKey, Name,
                                      get_connection()->get_client_ipv4(),
                                      SoftVer, proto_version, UserID);

            if (ClientID == 0) {
              // something goes wrong
              db->rollback();

            } else {
              client_enabled = true;

              if (AccessID == 0) {
                _accessid_enabled = false;
                _accessid_active = false;
                AccessID = db->get_client_access_id(
                    ClientID, &_accessid_enabled, &_accessid_active);

                if (AccessID) {
                  if (_accessid_enabled) {
                    accessid_enabled = true;
                  }
                  if (_accessid_active) {
                    accessid_active = true;
                  }
                }
              }
            }
          }

        } else if (_AccessID > 0 && register_client_d != NULL) {
          AccessID = _AccessID;
        }

        if (ClientID != 0) {
          if (!client_enabled) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_CLIENT_DISABLED;

          } else {
            if (do_update) {
              if (AccessID == 0 && pwd_is_set) {
                if (!is_superuser_authorized()) {
                  superuser_authorize(UserID, register_client_d->Email,
                                      register_client_d->Password, NULL);
                }

                if (is_superuser_authorized()) {
                  AccessID = db->get_access_id(UserID, true, true);

                  if (AccessID) {
                    accessid_enabled = true;
                    accessid_active = true;
                  }
                }
              }

              if (false ==
                  db->update_client(ClientID, AccessID, AuthKey, Name,
                                    get_connection()->get_client_ipv4(),
                                    SoftVer, proto_version)) {
                // something goes wrong
                ClientID = 0;
                db->rollback();
              }
            }

            if (ClientID) {
              db->commit();

              if (AccessID == 0) {
                resultcode = pwd_is_set
                                 ? SUPLA_RESULTCODE_BAD_CREDENTIALS
                                 : SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED;

              } else if (!accessid_enabled) {
                resultcode = SUPLA_RESULTCODE_ACCESSID_DISABLED;

              } else if (!accessid_active) {
                resultcode = SUPLA_RESULTCODE_ACCESSID_INACTIVE;

              } else {
                set_id(ClientID);
                setName(Name);
                setAccessID(AccessID);

                // Set the user before loading config
                set_user(supla_user::add_client(get_shared_ptr(), UserID));

                loadConfig();

                resultcode = SUPLA_RESULTCODE_TRUE;
                result = 1;
              }
            }
          }
        }
      }
    }

    delete db;
  }

  revoke_superuser_authorization();

  if (proto_version >= 17) {
    TSC_SuplaRegisterClientResult_C srcr = {};
    srcr.result_code = resultcode;
    srcr.ClientID = get_id();
    srcr.activity_timeout = get_connection()->get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srcr.ChannelGroupCount = cgroups->count();

    struct timeval now;
    gettimeofday(&now, NULL);
    srcr.serverUnixTimestamp = now.tv_sec;

    srpc_sc_async_registerclient_result_c(
        get_connection()->get_srpc_adapter()->get_srpc(), &srcr);
  } else if (proto_version >= 9) {
    TSC_SuplaRegisterClientResult_B srcr = {};
    srcr.result_code = resultcode;
    srcr.ClientID = get_id();
    srcr.activity_timeout = get_connection()->get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srcr.ChannelGroupCount = cgroups->count();
    srpc_sc_async_registerclient_result_b(
        get_connection()->get_srpc_adapter()->get_srpc(), &srcr);
  } else {
    TSC_SuplaRegisterClientResult srcr = {};
    srcr.result_code = resultcode;
    srcr.ClientID = get_id();
    srcr.activity_timeout = get_connection()->get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srpc_sc_async_registerclient_result(
        get_connection()->get_srpc_adapter()->get_srpc(), &srcr);
  }

  // !After srpc_async_registerclient_result
  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    remote_update_lists();
    supla_log(LOG_INFO, "Client registered. ClientSD: %i Protocol Version: %i",
              get_connection()->get_client_sd(),
              get_connection()->get_protocol_version());
  } else {
    usleep(2000000);
  }

  return result;
}

void supla_client::update_device_channels(int LocationID, int DeviceID) {
  if (locations->location_exists(LocationID)) {
    channels->update_device_channels(DeviceID);
    remote_update_lists();
  }
}

void supla_client::on_channel_value_changed(int DeviceId, int ChannelId,
                                            bool Extended) {
  channels->on_channel_value_changed(
      get_connection()->get_srpc_adapter()->get_srpc(), DeviceId, ChannelId,
      Extended);
  if (!Extended) {
    cgroups->on_channel_value_changed(
        get_connection()->get_srpc_adapter()->get_srpc(), DeviceId, ChannelId);
  }
}

void supla_client::remote_update_lists(void) {
  if (locations->remote_update(
          get_connection()->get_srpc_adapter()->get_srpc()))
    return;

  if (channels->remote_update(get_connection()->get_srpc_adapter()->get_srpc()))
    return;

  if (cgroups->remote_update(get_connection()->get_srpc_adapter()->get_srpc()))
    return;

  if (scenes->update_remote()) return;
}

void supla_client::loadConfig(void) {
  locations->load(get_id());
  channels->load();
  cgroups->load();
  scenes->load(get_user_id(), get_id());
}

void supla_client::get_next(void) { remote_update_lists(); }

void supla_client::call_event(TSC_SuplaEvent *event) {
  if (event != NULL && (event->Event != SUPLA_EVENT_SET_BRIDGE_VALUE_FAILED ||
                        event->SenderID == get_id())) {
    srpc_sc_async_event(get_connection()->get_srpc_adapter()->get_srpc(),
                        event);
  }
}

void supla_client::superuser_authorize(
    int UserID, const char Email[SUPLA_EMAIL_MAXSIZE],
    const char Password[SUPLA_PASSWORD_MAXSIZE], bool *connection_failed) {
  revoke_superuser_authorization();

  if (UserID == 0 || Email == NULL || Password == NULL ||
      strnlen(Email, SUPLA_EMAIL_MAXSIZE) == 0 ||
      strnlen(Password, SUPLA_PASSWORD_MAXSIZE) == 0) {
    return;
  }

  database *db = new database();

  if (db->connect() == true) {
    if (db->superuser_authorization(UserID, Email, Password)) {
      lck_lock(lck);
      superuser_authorized = true;
      lck_unlock(lck);
    }
  } else if (connection_failed) {
    *connection_failed = true;
  }

  delete db;
}

void supla_client::on_device_calcfg_result(int ChannelID,
                                           TDS_DeviceCalCfgResult *result) {
  if (result == NULL) return;

  TSC_DeviceCalCfgResult cresult;
  memset(&cresult, 0, sizeof(TSC_DeviceCalCfgResult));

  cresult.ChannelID = ChannelID;
  cresult.Command = result->Command;
  cresult.Result = result->Result;
  cresult.DataSize = result->DataSize > SUPLA_CALCFG_DATA_MAXSIZE
                         ? SUPLA_CALCFG_DATA_MAXSIZE
                         : result->DataSize;
  memcpy(cresult.Data, result->Data, SUPLA_CALCFG_DATA_MAXSIZE);

  srpc_sc_async_device_calcfg_result(
      get_connection()->get_srpc_adapter()->get_srpc(), &cresult);
}

void supla_client::on_device_channel_state_result(int ChannelID,
                                                  TDSC_ChannelState *state) {
  if (state == NULL) return;

  TDSC_ChannelState cstate;
  memcpy(&cstate, state, sizeof(TDSC_ChannelState));

  cstate.ChannelID = ChannelID;
  cstate.ReceiverID = get_id();

  srpc_csd_async_channel_state_result(
      get_connection()->get_srpc_adapter()->get_srpc(), &cstate);
}

void supla_client::set_channel_function(int ChannelId, int Func) {
  channels->set_channel_function(
      get_connection()->get_srpc_adapter()->get_srpc(), ChannelId, Func);
}

void supla_client::set_channel_function_result(
    TSC_SetChannelFunctionResult *result) {
  if (result == NULL) {
    return;
  }
  srpc_sc_async_set_channel_function_result(
      get_connection()->get_srpc_adapter()->get_srpc(), result);
}

void supla_client::set_channel_caption(int ChannelId, char *Caption) {
  channels->set_channel_caption(
      get_connection()->get_srpc_adapter()->get_srpc(), ChannelId, Caption);
}

void supla_client::set_location_caption(int LocationId, char *Caption) {
  locations->set_caption(LocationId, Caption);
  locations->remote_update(get_connection()->get_srpc_adapter()->get_srpc());
}

void supla_client::set_caption_result(TSC_SetCaptionResult *result,
                                      bool channel) {
  if (result == NULL) {
    return;
  }
  if (channel) {
    srpc_sc_async_set_channel_caption_result(
        get_connection()->get_srpc_adapter()->get_srpc(), result);
  } else {
    srpc_sc_async_set_location_caption_result(
        get_connection()->get_srpc_adapter()->get_srpc(), result);
  }
}

void supla_client::iterate() {
  channels->update_expired(get_connection()->get_srpc_adapter()->get_srpc());
}

unsigned _supla_int64_t supla_client::wait_time_usec() {
  unsigned _supla_int64_t time = channels->value_validity_time_usec();
  if (time > 0 && time < 120000000) {
    if (time < 1000000) {
      return 1000000;
    }
    return time + 500000;
  }

  return 120000000;
}
