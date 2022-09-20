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

#include "client/call_handler/abstract_register_client.h"

#include <string.h>
#include <unistd.h>

#include "log.h"

using std::weak_ptr;

supla_abstract_register_client::supla_abstract_register_client(void)
    : supla_abstract_register_object() {
  client_dao = nullptr;
  register_client_b = nullptr;
  register_client_d = nullptr;
  access_id = 0;
  client_id = 0;
  accessid_enabled = false;
  accessid_active = false;
  client_enabled = true;
  pwd_is_set = false;
  restrict_to_authentication = false;
  stay_connected = false;
  dont_send_result = false;
  resultcode = 0;
}

supla_abstract_register_client::~supla_abstract_register_client() {}

supla_abstract_client_dao *supla_abstract_register_client::get_client_dao(
    void) {
  return client_dao;
}

std::weak_ptr<supla_client> supla_abstract_register_client::get_client(void) {
  return client;
}

int supla_abstract_register_client::get_client_id(void) { return client_id; }

int supla_abstract_register_client::get_access_id(void) { return access_id; }

void supla_abstract_register_client::send_result(int resultcode) {
  this->resultcode = resultcode;

  revoke_superuser_authorization();

  if (get_dba()->is_connected()) {
    if (get_should_rollback()) {
      get_dba()->rollback();
    }

    if (!stay_connected) {
      get_dba()->disconnect();
    }
  }

  if (dont_send_result) {
    return;
  }

  struct timeval now;
  gettimeofday(&now, nullptr);

  if (get_srpc_adapter()->get_proto_version() >= 19) {
    TSC_SuplaRegisterClientResult_D srcr = {};
    srcr.result_code = resultcode;
    srcr.activity_timeout = get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.serverUnixTimestamp = now.tv_sec;

    if (resultcode == SUPLA_RESULTCODE_TRUE) {
      srcr.ClientID = client_id;
      srcr.LocationCount = get_location_count();
      srcr.ChannelCount = get_channel_count();
      srcr.ChannelGroupCount = get_channel_group_count();
      srcr.SceneCount = get_scene_count();
    }

    get_srpc_adapter()->sc_async_registerclient_result_d(&srcr);

  } else if (get_srpc_adapter()->get_proto_version() >= 17) {
    TSC_SuplaRegisterClientResult_C srcr = {};
    srcr.result_code = resultcode;
    srcr.activity_timeout = get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.serverUnixTimestamp = now.tv_sec;

    if (resultcode == SUPLA_RESULTCODE_TRUE) {
      srcr.ClientID = client_id;
      srcr.LocationCount = get_location_count();
      srcr.ChannelCount = get_channel_count();
      srcr.ChannelGroupCount = get_channel_group_count();
    }

    get_srpc_adapter()->sc_async_registerclient_result_c(&srcr);

  } else if (get_srpc_adapter()->get_proto_version() >= 9) {
    TSC_SuplaRegisterClientResult_B srcr = {};
    srcr.result_code = resultcode;
    srcr.activity_timeout = get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;

    if (resultcode == SUPLA_RESULTCODE_TRUE) {
      srcr.ClientID = client_id;
      srcr.LocationCount = get_location_count();
      srcr.ChannelCount = get_channel_count();
      srcr.ChannelGroupCount = get_channel_group_count();
    }

    get_srpc_adapter()->sc_async_registerclient_result_b(&srcr);

  } else {
    TSC_SuplaRegisterClientResult srcr = {};
    srcr.result_code = resultcode;
    srcr.activity_timeout = get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;

    if (resultcode == SUPLA_RESULTCODE_TRUE) {
      srcr.ClientID = client_id;
      srcr.LocationCount = get_location_count();
      srcr.ChannelCount = get_channel_count();
    }

    get_srpc_adapter()->sc_async_registerclient_result(&srcr);
  }

  // !After srpc_async_registerclient_result
  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    remote_update_lists();
    supla_log(LOG_INFO, "Client registered. ClientSD: %i Protocol Version: %i",
              get_client_sd(), get_srpc_adapter()->get_proto_version());
  } else {
    usleep(get_hold_time_on_failure_usec());
  }
}

bool supla_abstract_register_client::client_auth(void) {
  if (register_client_b != nullptr &&
      false == client_dao->access_id_auth(
                   access_id, register_client_b->AccessIDpwd, get_user_id_ptr(),
                   &accessid_enabled, &accessid_active)) {
    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  }

  if (register_client_d != nullptr &&
      !authkey_auth(get_guid(), register_client_d->Email, get_authkey(),
                    get_user_id_ptr())) {
    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  }

  if (get_user_id() == 0) {
    send_result(SUPLA_RESULTCODE_BAD_CREDENTIALS);
    return false;
  }

  return true;
}

bool supla_abstract_register_client::add_client(void) {
  if (pwd_is_set) {
    superuser_authorize(get_user_id(), register_client_d->Email,
                        register_client_d->Password);
  }

  if (!client_dao->get_client_reg_enabled(get_user_id()) &&
      !is_superuser_authorized()) {
    send_result(pwd_is_set ? SUPLA_RESULTCODE_BAD_CREDENTIALS
                           : SUPLA_RESULTCODE_REGISTRATION_DISABLED);
    return false;
  }

  if (client_dao->get_client_limit_left(get_user_id()) <= 0) {
    send_result(SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED);
    return false;
  }

  if (access_id == 0 && register_client_d != nullptr &&
      (client_dao->get_client_count(get_user_id()) == 0 ||
       is_superuser_authorized())) {
    access_id = client_dao->get_access_id(get_user_id(), true, true);

    if (access_id > 0) {
      accessid_enabled = true;
      accessid_active = true;
    } else {
      accessid_enabled = false;
      access_id = client_dao->get_access_id(get_user_id(), false, true);
      if (access_id > 0) {
        accessid_active = true;
      } else {
        accessid_active = false;
        access_id = client_dao->get_access_id(get_user_id(), false, false);
      }
    }
  }

  client_id = client_dao->add_client(
      access_id, get_guid(), get_authkey(), get_name(), get_client_ipv4(),
      get_softver(), get_srpc_adapter()->get_proto_version(), get_user_id());

  if (client_id == 0) {
    // something goes wrong
    send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
    return false;
  } else {
    client_enabled = true;

    if (access_id == 0) {
      bool _accessid_enabled = false;
      bool _accessid_active = false;
      access_id = client_dao->get_client_access_id(
          client_id, &_accessid_enabled, &_accessid_active);

      if (access_id) {
        if (_accessid_enabled) {
          accessid_enabled = true;
        }
        if (_accessid_active) {
          accessid_active = true;
        }
      }
    }
  }

  return true;
}

bool supla_abstract_register_client::update_client(void) {
  if (access_id == 0 && pwd_is_set) {
    if (!is_superuser_authorized()) {
      superuser_authorize(get_user_id(), register_client_d->Email,
                          register_client_d->Password);
    }

    if (is_superuser_authorized()) {
      access_id = client_dao->get_access_id(get_user_id(), true, true);

      if (access_id) {
        accessid_enabled = true;
        accessid_active = true;
      } else {
        pwd_is_set = false;
      }
    }
  }

  if (!client_dao->update_client(client_id, access_id, get_authkey(),
                                 get_name(), get_client_ipv4(), get_softver(),
                                 get_srpc_adapter()->get_proto_version())) {
    send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
    return false;
  }

  return true;
}

void supla_abstract_register_client::register_client(
    std::weak_ptr<supla_client> client,
    TCS_SuplaRegisterClient_B *register_client_b,
    TCS_SuplaRegisterClient_D *register_client_d,
    supla_abstract_srpc_adapter *srpc_adapter,
    supla_abstract_db_access_provider *dba,
    supla_abstract_connection_dao *conn_dao,
    supla_abstract_client_dao *client_dao, int client_sd, int client_ipv4,
    unsigned char activity_timeout) {
  this->client = client;
  this->register_client_b = register_client_b;
  this->register_client_d = register_client_d;
  access_id = 0;
  client_id = 0;
  accessid_enabled = false;
  accessid_active = false;
  client_enabled = true;
  pwd_is_set = false;
  resultcode = 0;

  set_srpc_adapter(srpc_adapter);
  set_dba(dba);
  set_conn_dao(conn_dao);
  this->client_dao = client_dao;
  set_client_sd(client_sd);
  set_client_ipv4(client_ipv4);
  set_activity_timeout(activity_timeout);

  set_should_rollback(true);

  if (register_client_b != nullptr) {
    set_guid(register_client_b->GUID);
    set_name(register_client_b->Name);
    access_id = register_client_b->AccessID;
    set_softver(register_client_b->SoftVer);
  } else {
    set_guid(register_client_d->GUID);
    set_name(register_client_d->Name);
    set_authkey(register_client_d->AuthKey);
    set_softver(register_client_d->SoftVer);
  }

  if (!is_valid_guid()) {
    send_result(SUPLA_RESULTCODE_GUID_ERROR);
    return;
  }

  if (register_client_d != nullptr && !is_valid_authkey()) {
    send_result(SUPLA_RESULTCODE_AUTHKEY_ERROR);
    return;
  }

  if (!dba->connect()) {
    send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
    return;
  }

  pwd_is_set = !restrict_to_authentication && register_client_d != nullptr &&
               strnlen(register_client_d->Password, SUPLA_PASSWORD_MAXSIZE) > 0;

  if (!client_auth()) {
    return;
  }

  _supla_int_t _access_id = access_id;

  client_id = client_dao->get_client_id(get_user_id(), get_guid());

  {
    bool _accessid_enabled = false;
    bool _accessid_active = false;

    if (client_id && !client_dao->get_client_variables(
                         client_id, &client_enabled, &_access_id,
                         &_accessid_enabled, &_accessid_active)) {
      supla_log(LOG_WARNING,
                "Unable to get variables for the client with id: %i",
                client_id);
      send_result(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE);
      return;
    }

    if (_accessid_enabled) accessid_enabled = true;
    if (_accessid_active) accessid_active = true;
  }

  if (!restrict_to_authentication) {
    dba->start_transaction();
  }

  if (client_id == 0) {
    if (restrict_to_authentication) {
      send_result(SUPLA_RESULTCODE_CLIENT_NOT_EXISTS);
      return;
    }

    if (!add_client()) {
      return;
    }
  } else {
    if (_access_id > 0 && register_client_d != nullptr) {
      access_id = _access_id;
    }

    if (!client_enabled) {
      send_result(SUPLA_RESULTCODE_CLIENT_DISABLED);
      return;
    } else if (!restrict_to_authentication && !update_client()) {
      return;
    }
  }

  if (!restrict_to_authentication) {
    dba->commit();
    set_should_rollback(false);
  }

  if (access_id == 0) {
    send_result(pwd_is_set ? SUPLA_RESULTCODE_BAD_CREDENTIALS
                           : SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED);
    return;
  }

  if (!accessid_enabled) {
    send_result(SUPLA_RESULTCODE_ACCESSID_DISABLED);
    return;
  }

  if (!accessid_active) {
    send_result(SUPLA_RESULTCODE_ACCESSID_INACTIVE);
    return;
  }

  if (!restrict_to_authentication) {
    on_registraction_success();
  }

  send_result(SUPLA_RESULTCODE_TRUE);
}

void supla_abstract_register_client::authenticate(
    std::weak_ptr<supla_client> client, TCS_ClientAuthorizationDetails *auth,
    supla_abstract_srpc_adapter *srpc_adapter,
    supla_abstract_db_access_provider *dba,
    supla_abstract_connection_dao *conn_dao,
    supla_abstract_client_dao *client_dao, bool stay_connected) {
  this->stay_connected = stay_connected;
  dont_send_result = true;
  restrict_to_authentication = true;

  TCS_SuplaRegisterClient_B register_client_b = {};
  TCS_SuplaRegisterClient_D register_client_d = {};
  bool email_auth = false;

  if (auth->Email[0] != 0) {
    memcpy(register_client_d.GUID, auth->GUID, SUPLA_GUID_SIZE);
    memcpy(register_client_d.AuthKey, auth->AuthKey, SUPLA_AUTHKEY_SIZE);
    memcpy(register_client_d.Email, auth->Email, SUPLA_EMAIL_MAXSIZE);
    email_auth = true;
  } else {
    register_client_b.AccessID = auth->AccessID;
    memcpy(register_client_b.GUID, auth->GUID, SUPLA_GUID_SIZE);
    memcpy(register_client_b.AccessIDpwd, auth->AccessIDpwd,
           SUPLA_ACCESSID_PWD_MAXSIZE);
  }

  register_client(client, email_auth ? nullptr : &register_client_b,
                  email_auth ? &register_client_d : nullptr, srpc_adapter, dba,
                  conn_dao, client_dao, 0, 0, 0);

  this->stay_connected = false;
  dont_send_result = false;
  restrict_to_authentication = false;
}

__useconds_t supla_abstract_register_client::get_hold_time_on_failure_usec(
    void) {
  return supla_abstract_register_object::get_hold_time_on_failure_usec();
}

int supla_abstract_register_client::get_result_code(void) { return resultcode; }
