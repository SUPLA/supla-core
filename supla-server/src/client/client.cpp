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

bool supla_client::is_superuser_authorized(void) {
  bool result = false;
  lck_lock(lck);
  result = superuser_authorized;
  lck_unlock(lck);

  return result;
}

char supla_client::register_client(TCS_SuplaRegisterClient_B *register_client_b,
                                   TCS_SuplaRegisterClient_C *register_client_c,
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
    GUID = register_client_c->GUID;
    Name = register_client_c->Name;
    AuthKey = register_client_c->AuthKey;
    SoftVer = register_client_c->SoftVer;
  }

  if (!setGUID(GUID)) {
    resultcode = SUPLA_RESULTCODE_GUID_ERROR;

  } else if (register_client_c != NULL &&
             !setAuthKey(register_client_c->AuthKey)) {
    resultcode = SUPLA_RESULTCODE_AUTHKEY_ERROR;

  } else if (register_client_b == NULL && register_client_c == NULL) {
    resultcode = SUPLA_RESULTCODE_UNSUPORTED;

  } else {
    database *db = new database();

    if (db->connect() == true) {
      int UserID = 0;
      bool accessid_enabled = false;

      if (register_client_b != NULL &&
          false == db->accessid_auth(AccessID, register_client_b->AccessIDpwd,
                                     &UserID, &accessid_enabled)) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (register_client_c != NULL &&
                 false == db->client_authkey_auth(
                              GUID, register_client_c->Email,
                              register_client_c->AuthKey, &UserID)) {
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

          if (false == db->get_client_reg_enabled(UserID)) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_REGISTRATION_DISABLED;

          } else if (db->get_client_limit_left(UserID) <= 0) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED;

          } else {
            if (AccessID == 0 && register_client_c != NULL &&
                db->get_client_count(UserID) == 0) {
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

        } else if (_AccessID > 0 && register_client_c != NULL) {
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
                resultcode = SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED;

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

  if (proto_version >= 9) {
    TSC_SuplaRegisterClientResult_B srcr;
    srcr.result_code = resultcode;
    srcr.ClientID = getID();
    srcr.activity_timeout = getSvrConn()->GetActivityTimeout();
    srcr.version_min = SUPLA_PROTO_VERSION;
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
    srcr.version_min = SUPLA_PROTO_VERSION;
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
  srpc_sc_async_event(getSvrConn()->srpc(), event);
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
      }
    } else {
      result.ResultCode = SUPLA_OAUTH_TEMPORARILY_UNAVAILABLE;
    }

    delete db;
  }

  srpc_cs_async_oauth_token_request_result(getSvrConn()->srpc(), &result);
}

void supla_client::superuser_authorization_request(
    TCS_SuperUserAuthorizationRequest *request) {
  if (request == NULL) {
    lck_lock(lck);
    superuser_authorized = false;
    lck_unlock(lck);
    return;
  }

  TSC_SuperUserAuthorizationResult result;
  memset(&result, 0, sizeof(TSC_SuperUserAuthorizationResult));
  result.Result = SUPLA_RESULTCODE_UNAUTHORIZED;

  if (getUser()) {
    database *db = new database();

    if (db->connect() == true) {
      if (db->superuser_authorization(getUser()->getUserID(), request->Email,
                                      request->Password)) {
        lck_lock(lck);
        superuser_authorized = true;
        lck_unlock(lck);

        result.Result = SUPLA_RESULTCODE_AUTHORIZED;
      }
    } else {
      result.Result = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
    }

    delete db;
  }

  srpc_sc_async_superuser_authorization_result(getSvrConn()->srpc(), &result);
}

void supla_client::device_calcfg_request(TCS_DeviceCalCfgRequest_B *request) {
  if (request->Target == SUPLA_TARGET_CHANNEL) {
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
