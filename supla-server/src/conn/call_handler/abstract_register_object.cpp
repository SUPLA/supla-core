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

#include "conn/call_handler/abstract_register_object.h"

#include <string.h>

supla_ch_abstract_register_object::supla_ch_abstract_register_object(void) {
  guid = nullptr;
  authkey = nullptr;
  name = nullptr;
  softver = nullptr;
  user_id = 0;

  should_rollback = true;
  srpc_adapter = nullptr;
  dba = nullptr;
  conn_dao = nullptr;
  client_sd = -1;
  client_ipv4 = 0;
  activity_timeout = 0;
  hold_time_on_failure_usec = 2000000;
}

supla_ch_abstract_register_object::~supla_ch_abstract_register_object() {}

bool supla_ch_abstract_register_object::authkey_auth(
    const char guid[SUPLA_GUID_SIZE], const char email[SUPLA_EMAIL_MAXSIZE],
    const char authkey[SUPLA_AUTHKEY_SIZE], int *user_id) {
  return get_authkey_cache()->authkey_auth(
      guid, email, authkey, user_id,
      [this, guid, email, authkey, user_id]() -> bool {
        int id = 0;
        int _user_id = get_user_id_by_email(email);

        if (_user_id == 0) {
          return false;
        }

        if (!get_object_id(_user_id, guid, &id)) {
          return false;
        }

        if (id == 0) {
          // Yes. When client/device not exists then is authorized
          *user_id = _user_id;
          return true;
        }

        bool is_null = false;
        char authkey_hash[BCRYPT_HASH_MAXSIZE] = {};

        if (!get_authkey_hash(id, authkey_hash, &is_null)) {
          return false;
        }

        if (is_null) {  // Yes. When is null then is authorized
          *user_id = _user_id;
          return true;
        }

        char authkey_hex[SUPLA_AUTHKEY_HEXSIZE];
        memset(authkey_hex, 0, SUPLA_AUTHKEY_HEXSIZE);

        st_authkey2hex(authkey_hex, authkey);

        if (st_bcrypt_check(authkey_hex, authkey_hash,
                            strnlen(authkey_hash, BCRYPT_HASH_MAXSIZE))) {
          *user_id = _user_id;
          return true;
        }

        return false;
      });
}

bool supla_ch_abstract_register_object::is_valid_guid(void) {
  char _guid[SUPLA_GUID_SIZE] = {};
  return memcmp(_guid, guid, SUPLA_GUID_SIZE) != 0;
}

bool supla_ch_abstract_register_object::is_valid_authkey(void) {
  char _authkey[SUPLA_AUTHKEY_SIZE] = {};
  return memcmp(_authkey, authkey, SUPLA_AUTHKEY_SIZE) != 0;
}

void supla_ch_abstract_register_object::set_guid(char *guid) {
  this->guid = guid;
}

char *supla_ch_abstract_register_object::get_guid(void) { return guid; }

void supla_ch_abstract_register_object::set_authkey(char *authkey) {
  this->authkey = authkey;
}

char *supla_ch_abstract_register_object::get_authkey(void) { return authkey; }

void supla_ch_abstract_register_object::set_name(char *name) {
  this->name = name;
}

char *supla_ch_abstract_register_object::get_name(void) { return name; }

void supla_ch_abstract_register_object::set_softver(char *softver) {
  this->softver = softver;
}

char *supla_ch_abstract_register_object::get_softver(void) { return softver; }

void supla_ch_abstract_register_object::set_client_sd(int client_sd) {
  this->client_sd = client_sd;
}

int supla_ch_abstract_register_object::get_client_sd(void) { return client_sd; }

void supla_ch_abstract_register_object::set_client_ipv4(int client_ipv4) {
  this->client_ipv4 = client_ipv4;
}

int supla_ch_abstract_register_object::get_client_ipv4(void) {
  return client_ipv4;
}

void supla_ch_abstract_register_object::set_activity_timeout(
    unsigned char activity_timeout) {
  this->activity_timeout = activity_timeout;
}

unsigned char supla_ch_abstract_register_object::get_activity_timeout(void) {
  return activity_timeout;
}

void supla_ch_abstract_register_object::set_user_id(int user_id) {
  this->user_id = user_id;
}

int supla_ch_abstract_register_object::get_user_id() { return user_id; }

int *supla_ch_abstract_register_object::get_user_id_ptr() { return &user_id; }

void supla_ch_abstract_register_object::set_should_rollback(
    bool should_rollback) {
  this->should_rollback = should_rollback;
}

bool supla_ch_abstract_register_object::get_should_rollback(void) {
  return should_rollback;
}

void supla_ch_abstract_register_object::set_srpc_adapter(
    supla_abstract_srpc_adapter *srpc_adapter) {
  this->srpc_adapter = srpc_adapter;
}

supla_abstract_srpc_adapter *
supla_ch_abstract_register_object::get_srpc_adapter(void) {
  return srpc_adapter;
}

void supla_ch_abstract_register_object::set_dba(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_abstract_db_access_provider *supla_ch_abstract_register_object::get_dba(
    void) {
  return dba;
}

void supla_ch_abstract_register_object::set_conn_dao(
    supla_abstract_connection_dao *conn_dao) {
  this->conn_dao = conn_dao;
}

supla_abstract_connection_dao *supla_ch_abstract_register_object::get_conn_dao(
    void) {
  return conn_dao;
}

__useconds_t supla_ch_abstract_register_object::get_hold_time_on_failure_usec(
    void) {
  return hold_time_on_failure_usec;
}
