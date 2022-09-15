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

#include "client/client_dao.h"

#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "proto.h"
#include "svrcfg.h"
#include "tools.h"

supla_client_dao::supla_client_dao(supla_abstract_db_access_provider *dba)
    : supla_abstract_client_dao() {
  this->dba = dba;
}

supla_client_dao::~supla_client_dao() {}

bool supla_client_dao::access_id_auth(int access_id, char *access_id_pwd,
                                      int *user_id, bool *is_enabled,
                                      bool *is_active) {
  if (access_id == 0 || access_id_pwd == nullptr || user_id == nullptr ||
      is_enabled == nullptr || is_active == nullptr ||
      strnlen(access_id_pwd, SUPLA_ACCESSID_PWD_MAXSIZE) < 1)
    return false;

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&access_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)access_id_pwd;
  pbind[1].buffer_length = strnlen(access_id_pwd, SUPLA_LOCATION_PWD_MAXSIZE);

  int _is_enabled = 0;
  int _is_active = 0;

  const char query[] =
      "SELECT id, user_id, enabled, is_now_active FROM "
      "`supla_v_accessid_active` WHERE id = ? AND password = ?";

  int _ID = 0;
  MYSQL_STMT *stmt = nullptr;
  if (!dba->stmt_get_int((void **)&stmt, &_ID, user_id, &_is_enabled,
                         &_is_active, query, pbind, 2)) {
    _ID = 0;
    _is_enabled = 0;
    _is_active = 0;
  }

  *is_enabled = _is_enabled == 1;
  *is_active = _is_active == 1;

  return _ID != 0;
}

int supla_client_dao::oauth_add_client_id(void) {
  char random_id[51];
  char secret[51];

  st_random_alpha_string(random_id, 51);
  st_random_alpha_string(secret, 51);

  for (int a = 0; a < 51; a++) {
    random_id[a] = tolower(random_id[a]);
    secret[a] = tolower(secret[a]);
  }

  char sql[] = "CALL `supla_oauth_add_client_for_app`(?,?,@id)";

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)random_id;
  pbind[0].buffer_length = strnlen(random_id, 50);

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)secret;
  pbind[1].buffer_length = strnlen(secret, 50);

  return dba->add_by_proc_call(sql, pbind, 2);
}

int supla_client_dao::oauth_get_client_id(bool create) {
  int id = 0;
  MYSQL_STMT *stmt = nullptr;

  if (!dba->stmt_get_int(
          (void **)&stmt, &id, nullptr, nullptr, nullptr,
          "SELECT `id` FROM `supla_oauth_clients` WHERE `type` = 2 LIMIT 1",
          nullptr, 0)) {
    id = 0;
  }

  if (id == 0 && create) {
    id = oauth_add_client_id();
  }

  return id;
}

bool supla_client_dao::oauth_get_token(TSC_OAuthToken *token, int user_id,
                                       int access_id,
                                       bool *storage_connect_error) {
  // TODO(anyone): This code has been ported from database.cpp. The business
  // logic code should be moved outside of the DAO.

  if (token == nullptr) {
    return false;
  }

  memset(token, 0, sizeof(TSC_OAuthToken));

  if (oauth_get_client_id(true) == 0 || svrcfg_oauth_url_base64 == nullptr ||
      svrcfg_oauth_url_base64_len <= 0 ||
      svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 2 >
          SUPLA_OAUTH_TOKEN_MAXSIZE) {
    return false;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    if (storage_connect_error) {
      *storage_connect_error = true;
    }
    return false;
  }

  st_random_alpha_string(token->Token, CFG_OAUTH_TOKEN_SIZE + 1);
  token->Token[CFG_OAUTH_TOKEN_SIZE] = '.';

  memcpy(&token->Token[CFG_OAUTH_TOKEN_SIZE + 1], svrcfg_oauth_url_base64,
         svrcfg_oauth_url_base64_len);
  token->Token[svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 1] = 0;
  token->TokenSize = svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 2;
  token->ExpiresIn = (unsigned)scfg_int(CFG_OAUTH_TOKEN_LIFETIME);

  int ExpiresIn = token->ExpiresIn + (unsigned)time(nullptr);

  char sql[] = "CALL `supla_oauth_add_token_for_app`(?,?,?,?,@id)";

  MYSQL_BIND pbind[4];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)token->Token;
  pbind[1].buffer_length = token->TokenSize - 1;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&ExpiresIn;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&access_id;

  bool result = dba->add_by_proc_call(sql, pbind, 4) > 0;

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

bool supla_client_dao::set_reg_enabled(int user_id, int device_reg_time_sec,
                                       int client_reg_time_sec) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  char sql[100] = {};
  snprintf(sql, sizeof(sql),
           "CALL `supla_set_registration_enabled`(%i, %i, %i)", user_id,
           device_reg_time_sec, client_reg_time_sec);

  bool result = dba->query(sql, true) == 0;

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

bool supla_client_dao::get_client_variables(int client_id, bool *client_enabled,
                                            int *access_id,
                                            bool *accessid_enabled,
                                            bool *accessid_active) {
  if (client_id == 0) return false;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&client_id;

  int _client_enabled = 0;
  int _access_id = 0;
  int _accessid_enabled = 0;
  int _accessid_active = 0;

  MYSQL_STMT *stmt = nullptr;

  if (dba->stmt_get_int(
          (void **)&stmt, &_client_enabled, &_access_id, &_accessid_enabled,
          &_accessid_active,
          "SELECT CAST(c.`enabled` AS unsigned integer), "
          "IFnullptr(c.access_id, "
          "0), IFnullptr(CAST(a.`enabled` AS unsigned integer), 0), "
          "IFnullptr(CAST(a.`is_now_active` AS unsigned integer), 0) FROM "
          "supla_client c LEFT JOIN supla_v_accessid_active a ON a.id = "
          "c.access_id WHERE c.id = ?",
          &pbind, 1)) {
    *client_enabled = _client_enabled == 1;
    *access_id = _access_id;
    *accessid_enabled = _accessid_enabled == 1;
    *accessid_active = _accessid_active == 1;
    return true;
  }

  return false;
}

bool supla_client_dao::get_client_id(int user_id,
                                     const char guid[SUPLA_GUID_SIZE],
                                     int *id) {
  if (!id) {
    return false;
  }

  // We intentionally use IFnullptr(MIN (id), 0) so that the query result always
  // contains one row. Otherwise, we assume that an error has occurred.

  const char query[] =
      "SELECT IFnullptr(MIN(id), 0) FROM supla_client WHERE user_id = ? AND "
      "guid = unhex(?)";

  MYSQL_STMT *stmt = nullptr;

  char guidhex[SUPLA_GUID_HEXSIZE];
  st_guid2hex(guidhex, guid);

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)guidhex;
  pbind[1].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  if (dba->stmt_get_int((void **)&stmt, id, nullptr, nullptr, nullptr, query,
                        pbind, 2)) {
    return true;
  }

  *id = 0;
  return false;
}

int supla_client_dao::get_client_id(int user_id,
                                    const char guid[SUPLA_GUID_SIZE]) {
  int result = 0;
  if (get_client_id(user_id, guid, &result)) {
    return result;
  }

  return 0;
}

int supla_client_dao::get_client_access_id(int client_id,
                                           bool *accessid_enabled,
                                           bool *accessid_active) {
  MYSQL_STMT *stmt = nullptr;
  int result = 0;

  MYSQL_BIND pbind = {};

  int _accessid_enabled = 0;
  int _accessid_active = 0;

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&client_id;

  if (!dba->stmt_get_int(
          (void **)&stmt, &result, &_accessid_enabled, &_accessid_active,
          nullptr,
          "SELECT c.access_id, a.enabled, a.is_now_active FROM "
          "`supla_client` c JOIN `supla_v_accessid_active` a ON a.id "
          "= c.access_id WHERE c.id = ?",
          &pbind, 1)) {
    return 0;
  }

  if (accessid_enabled) {
    *accessid_enabled = _accessid_enabled == 1;
  }

  if (accessid_active) {
    *accessid_active = _accessid_active == 1;
  }

  return result;
}

bool supla_client_dao::get_client_reg_enabled(int user_id) {
  const char query[] =
      "SELECT COUNT(*) FROM `supla_user` WHERE id = ? AND client_reg_enabled "
      "IS NOT nullptr AND client_reg_enabled >= UTC_TIMESTAMP()";

  return dba->get_count(user_id, query) > 0 ? true : false;
}

int supla_client_dao::get_client_limit_left(int user_id) {
  return dba->get_int(
      user_id, 0,
      "SELECT IFnullptr(limit_client, 0) - IFnullptr(( SELECT COUNT(*) "
      "FROM supla_client WHERE user_id = supla_user.id ), 0) FROM "
      "supla_user WHERE id = ?");
}

int supla_client_dao::get_client_count(int user_id) {
  return dba->get_count(user_id,
                        "SELECT COUNT(*) FROM supla_client WHERE user_id = ?");
}

int supla_client_dao::get_access_id(int user_id, bool enabled, bool active) {
  MYSQL_STMT *stmt = nullptr;
  int result = 0;
  int _enabled = enabled ? 1 : 0;
  int _active = active ? 1 : 0;

  MYSQL_BIND pbind[3] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&_enabled;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&_active;

  if (!dba->stmt_get_int(
          (void **)&stmt, &result, nullptr, nullptr, nullptr,
          "SELECT id FROM `supla_v_accessid_active` WHERE user_id = "
          "? AND enabled = ? AND is_now_active = ? LIMIT 1",
          pbind, 3)) {
    result = 0;
  }

  return result;
}

int supla_client_dao::add_client(int access_id, const char *guid,
                                 const char *authkey, const char *name,
                                 unsigned int ipv4, const char *softver,
                                 int proto_version, int user_id) {
  int client_id = 0;

  char *authkey_hash_hex = nullptr;

  MYSQL_BIND pbind[8] = {};

  char guid_hex[SUPLA_GUID_HEXSIZE];
  st_guid2hex(guid_hex, guid);

  if (access_id == 0) {
    pbind[0].buffer_type = MYSQL_TYPE_NULL;
  } else {
    pbind[0].buffer_type = MYSQL_TYPE_LONG;
    pbind[0].buffer = (char *)&access_id;
  }

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)guid_hex;
  pbind[1].buffer_length = SUPLA_GUID_HEXSIZE - 1;

  pbind[2].buffer_type = MYSQL_TYPE_STRING;
  pbind[2].buffer = (char *)name;
  pbind[2].buffer_length = strnlen(name, SUPLA_CLIENT_NAME_MAXSIZE);

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&ipv4;
  pbind[3].is_unsigned = true;

  pbind[4].buffer_type = MYSQL_TYPE_STRING;
  pbind[4].buffer = (char *)softver;
  pbind[4].buffer_length = strnlen(softver, SUPLA_SOFTVER_MAXSIZE);

  pbind[5].buffer_type = MYSQL_TYPE_LONG;
  pbind[5].buffer = (char *)&proto_version;

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&user_id;

  if (authkey == nullptr) {
    pbind[7].buffer_type = MYSQL_TYPE_NULL;
  } else {
    authkey_hash_hex = st_get_authkey_hash_hex(authkey);

    if (authkey_hash_hex == nullptr) return 0;

    pbind[7].buffer_type = MYSQL_TYPE_STRING;
    pbind[7].buffer = (char *)authkey_hash_hex;
    pbind[7].buffer_length = strnlen(authkey_hash_hex, BCRYPT_HASH_MAXSIZE * 2);
  }

  const char sql[] =
      "CALL "
      "`supla_add_client`(?,unhex(?),?,?,?,?,?,unhex(?),@id)";

  client_id = dba->add_by_proc_call(sql, pbind, 8);

  if (authkey_hash_hex) {
    free(authkey_hash_hex);
    authkey_hash_hex = nullptr;
  }

  if (client_id != 0) {
    on_newclient(client_id);
  }

  return client_id;
}

bool supla_client_dao::on_newclient(int client_id) {
  char sql[51] = {};
  snprintf(sql, sizeof(sql), "CALL `supla_on_newclient`(%i)", client_id);

  return dba->query(sql, true) == 0;
}

bool supla_client_dao::update_client(int client_id, int access_id,
                                     const char *authkey, const char *name,
                                     unsigned int ipv4, const char *softver,
                                     int proto_version) {
  bool result = false;

  MYSQL_BIND pbind[7] = {};

  char *authkey_hash_hex = nullptr;

  if (access_id == 0) {
    pbind[0].buffer_type = MYSQL_TYPE_NULL;
  } else {
    pbind[0].buffer_type = MYSQL_TYPE_LONG;
    pbind[0].buffer = (char *)&access_id;
  }

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)name;
  pbind[1].buffer_length = strnlen(name, SUPLA_CLIENT_NAME_MAXSIZE);

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&ipv4;
  pbind[2].is_unsigned = true;

  pbind[3].buffer_type = MYSQL_TYPE_STRING;
  pbind[3].buffer = (char *)softver;
  pbind[3].buffer_length = strnlen(softver, SUPLA_SOFTVER_MAXSIZE);

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&proto_version;

  if (authkey == nullptr) {
    pbind[5].buffer_type = MYSQL_TYPE_NULL;
  } else {
    authkey_hash_hex = st_get_authkey_hash_hex(authkey);

    if (authkey_hash_hex == nullptr) return 0;

    pbind[5].buffer_type = MYSQL_TYPE_STRING;
    pbind[5].buffer = (char *)authkey_hash_hex;
    pbind[5].buffer_length = strnlen(authkey_hash_hex, BCRYPT_HASH_MAXSIZE * 2);
  }

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&client_id;

  const char sql[] = "CALL `supla_update_client`(?,?,?,?,?,unhex(?),?)";

  MYSQL_STMT *stmt = nullptr;
  if (dba->stmt_execute((void **)&stmt, sql, pbind, 7, true)) {
    result = true;
  }

  if (stmt != nullptr) mysql_stmt_close(stmt);

  if (authkey_hash_hex) {
    free(authkey_hash_hex);
    authkey_hash_hex = nullptr;
  }

  return result;
}
