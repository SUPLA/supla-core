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

#include "client.h"
#include "clientlocation.h"
#include "database.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "srpc.h"
#include "user.h"

supla_client::supla_client(serverconnection *svrconn) : cdbase(svrconn) {
  this->locations = new supla_client_locations();
  this->channels = new supla_client_channels(this);
  this->cgroups = new supla_client_channelgroups(this);
  this->name[0] = 0;
  this->superuser_authorized = false;
  this->access_id = 0;
}

supla_client::~supla_client() {
  delete cgroups;
  delete channels;
  delete locations;
}

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

  if (!setGUID(GUID)) {
    resultcode = SUPLA_RESULTCODE_GUID_ERROR;

  } else if (register_client_d != NULL &&
             !setAuthKey(register_client_d->AuthKey)) {
    resultcode = SUPLA_RESULTCODE_AUTHKEY_ERROR;

  } else if (register_client_b == NULL && register_client_d == NULL) {
    resultcode = SUPLA_RESULTCODE_UNSUPORTED;

  } else {
    database *db = new database();

    if (db->connect() == true) {
      int UserID = 0;
      bool pwd_is_set = false;
      bool accessid_enabled = false;

      if (register_client_b != NULL &&
          false == db->accessid_auth(AccessID, register_client_b->AccessIDpwd,
                                     &UserID, &accessid_enabled)) {
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

        db->start_transaction();

        int ClientID =
            db->get_client(db->get_client_id(UserID, GUID), &client_enabled,
                           &_AccessID, &_accessid_enabled);

        if (_accessid_enabled) accessid_enabled = true;

        if (ClientID == 0) {
          do_update = false;
          pwd_is_set =
              register_client_d != NULL &&
              strnlen(register_client_d->Password, SUPLA_PASSWORD_MAXSIZE) > 0;

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
              AccessID = db->get_access_id(UserID, true);

              if (AccessID > 0) {
                accessid_enabled = true;
              } else {
                accessid_enabled = false;
                AccessID = db->get_access_id(UserID, false);
              }
            }

            ClientID = db->add_client(AccessID, GUID, AuthKey, Name,
                                      getSvrConn()->getClientIpv4(), SoftVer,
                                      proto_version, UserID);

            if (ClientID == 0) {
              // something goes wrong
              db->rollback();

            } else {
              client_enabled = true;

              if (AccessID == 0) {
                _accessid_enabled = false;
                AccessID =
                    db->get_client_access_id(ClientID, &_accessid_enabled);

                if (AccessID && _accessid_enabled) {
                  accessid_enabled = true;
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
              if (false == db->update_client(ClientID, AccessID, AuthKey, Name,
                                             getSvrConn()->getClientIpv4(),
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

              } else {
                setID(ClientID);
                setName(Name);
                setAccessID(AccessID);

                loadConfig();

                resultcode = SUPLA_RESULTCODE_TRUE;
                result = 1;
                setUser(supla_user::add_client(this, UserID));
              }
            }
          }
        }
      }
    }

    delete db;
  }

  revoke_superuser_authorization();

  if (proto_version >= 9) {
    TSC_SuplaRegisterClientResult_B srcr;
    srcr.result_code = resultcode;
    srcr.ClientID = getID();
    srcr.activity_timeout = getSvrConn()->GetActivityTimeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srcr.ChannelGroupCount = cgroups->count();
    srcr.Flags = 0;
    srpc_sc_async_registerclient_result_b(getSvrConn()->srpc(), &srcr);
  } else {
    TSC_SuplaRegisterClientResult srcr;
    srcr.result_code = resultcode;
    srcr.ClientID = getID();
    srcr.activity_timeout = getSvrConn()->GetActivityTimeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srpc_sc_async_registerclient_result(getSvrConn()->srpc(), &srcr);
  }

  // !After srpc_async_registerclient_result
  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    remote_update_lists();
    supla_log(LOG_INFO, "Client registered. ClientSD: %i Protocol Version: %i",
              getSvrConn()->getClientSD(), getSvrConn()->getProtocolVersion());
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
  channels->on_channel_value_changed(getSvrConn()->srpc(), DeviceId, ChannelId,
                                     Extended);
  if (!Extended) {
    cgroups->on_channel_value_changed(getSvrConn()->srpc(), DeviceId,
                                      ChannelId);
  }
}

void supla_client::remote_update_lists(void) {
  if (locations->remote_update(getSvrConn()->srpc())) return;

  if (channels->remote_update(getSvrConn()->srpc())) return;

  if (cgroups->remote_update(getSvrConn()->srpc())) return;
}

void supla_client::loadConfig(void) {
  locations->load(getID());
  channels->load();
  cgroups->load();
}

void supla_client::get_next(void) { remote_update_lists(); }

void supla_client::set_device_channel_new_value(
    TCS_SuplaChannelNewValue_B *channel_new_value) {
  channels->set_device_channel_new_value(channel_new_value);
}

void supla_client::set_new_value(TCS_SuplaNewValue *new_value) {
  if (new_value->Target == SUPLA_TARGET_CHANNEL) {
    channels->set_device_channel_new_value(new_value);
  } else if (new_value->Target == SUPLA_TARGET_GROUP) {
    cgroups->set_device_channel_new_value(new_value);
  }
}

void supla_client::call_event(TSC_SuplaEvent *event) {
  if (event != NULL && (event->Event != SUPLA_EVENT_SET_BRIDGE_VALUE_FAILED ||
                        event->SenderID == getID())) {
    srpc_sc_async_event(getSvrConn()->srpc(), event);
  }
}

void supla_client::oauth_token_request(void) {
  TSC_OAuthTokenRequestResult result;
  memset(&result, 0, sizeof(TSC_OAuthTokenRequestResult));

  int AccessID = getAccessID();

  result.ResultCode = SUPLA_OAUTH_RESULTCODE_ERROR;

  if (getUser() && AccessID) {
    database *db = new database();

    if (db->connect() == true) {
      if (db->oauth_get_token(&result.Token, getUser()->getUserID(),
                              AccessID)) {
        result.ResultCode = SUPLA_OAUTH_RESULTCODE_SUCCESS;
      } else {
        memset(&result.Token, 0, sizeof(TSC_OAuthToken));
      }
    } else {
      result.ResultCode = SUPLA_OAUTH_TEMPORARILY_UNAVAILABLE;
    }

    delete db;
  }

  srpc_cs_async_oauth_token_request_result(getSvrConn()->srpc(), &result);
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

void supla_client::send_superuser_authorization_result(
    bool *connection_failed) {
  TSC_SuperUserAuthorizationResult result;
  memset(&result, 0, sizeof(TSC_SuperUserAuthorizationResult));
  if (is_superuser_authorized()) {
    result.Result = SUPLA_RESULTCODE_AUTHORIZED;
  } else if (connection_failed && *connection_failed) {
    result.Result = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
  } else {
    result.Result = SUPLA_RESULTCODE_UNAUTHORIZED;
  }

  srpc_sc_async_superuser_authorization_result(getSvrConn()->srpc(), &result);
}

void supla_client::superuser_authorization_request(
    TCS_SuperUserAuthorizationRequest *request) {
  bool connection_failed = false;

  superuser_authorize(getUserID(), request ? request->Email : NULL,
                      request ? request->Password : NULL, &connection_failed);

  send_superuser_authorization_result(&connection_failed);
}

void supla_client::device_calcfg_request(TCS_DeviceCalCfgRequest_B *request) {
  if (request->Target == SUPLA_TARGET_CHANNEL ||
      request->Target == SUPLA_TARGET_IODEVICE) {
    channels->device_calcfg_request(request);
  } else if (request->Target == SUPLA_TARGET_GROUP) {
    cgroups->device_calcfg_request(request);
  }
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

  srpc_sc_async_device_calcfg_result(getSvrConn()->srpc(), &cresult);
}

void supla_client::device_get_channel_state(TCSD_ChannelStateRequest *request) {
  channels->device_get_channel_state(request);
}

void supla_client::on_device_channel_state_result(int ChannelID,
                                                  TDSC_ChannelState *state) {
  if (state == NULL) return;

  TDSC_ChannelState cstate;
  memcpy(&cstate, state, sizeof(TDSC_ChannelState));

  cstate.ChannelID = ChannelID;
  cstate.ReceiverID = getID();

  srpc_csd_async_channel_state_result(getSvrConn()->srpc(), &cstate);
}

void supla_client::get_channel_basic_cfg(TCS_ChannelBasicCfgRequest *request) {
  if (request == NULL) return;
  channels->get_channel_basic_cfg(getSvrConn()->srpc(), request);
}

void supla_client::set_channel_function(int ChannelId, int Func) {
  channels->set_channel_function(ChannelId, Func);
}

void supla_client::set_channel_function_request(TCS_SetChannelFunction *func) {
  getUser()->set_channel_function(this, func);
}

void supla_client::set_channel_function_result(
    TSC_SetChannelFunctionResult *result) {
  if (result == NULL) {
    return;
  }
  srpc_sc_async_set_channel_function_result(getSvrConn()->srpc(), result);
}

void supla_client::set_channel_caption(int ChannelId, char *Caption) {
  channels->set_channel_caption(ChannelId, Caption);
}

void supla_client::set_channel_caption_request(TCS_SetChannelCaption *caption) {
  getUser()->set_channel_caption(this, caption);
}

void supla_client::set_channel_caption_result(
    TSC_SetChannelCaptionResult *result) {
  if (result == NULL) {
    return;
  }
  srpc_sc_async_set_channel_caption_result(getSvrConn()->srpc(), result);
}

void supla_client::iterate() { channels->update_expired(getSvrConn()->srpc()); }

unsigned _supla_int64_t supla_client::waitTimeUSec() {
  unsigned _supla_int64_t time = channels->value_validity_time_usec();
  if (time > 0 && time < 120000000) {
    if (time < 1000000) {
      return 1000000;
    }
    return time + 500000;
  }

  return 120000000;
}
