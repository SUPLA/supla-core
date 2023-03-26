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

#include "alexa_credentials_dao.h"

#include <mysql.h>

#include "log.h"

using std::string;

#define ALEXA_TOKEN_MAXSIZE 1024
#define ALEXA_REGION_MAXSIZE 5

supla_amazon_alexa_credentials_dao::supla_amazon_alexa_credentials_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_amazon_alexa_credentials_dao::~supla_amazon_alexa_credentials_dao() {}

bool supla_amazon_alexa_credentials_dao::get(int user_id, string *access_token,
                                             string *refresh_token,
                                             int *expires_in, string *region) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  bool result = false;

  char sql[] =
      "SELECT `access_token`, `refresh_token`, TIMESTAMPDIFF(SECOND, "
      "UTC_TIMESTAMP(), expires_at) `expires_in`, `region` "
      "FROM "
      "`supla_amazon_alexa` WHERE user_id = ? AND LENGTH(access_token) > 0";

  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind[4] = {};

    char _access_token[ALEXA_TOKEN_MAXSIZE] = {};
    unsigned long access_token_size = 0;
    my_bool access_token_is_null = true;

    char _refresh_token[ALEXA_TOKEN_MAXSIZE] = {};
    unsigned long refresh_token_size = 0;
    my_bool refresh_token_is_null = true;

    char _region[ALEXA_REGION_MAXSIZE] = {};
    unsigned long region_size = 0;
    my_bool region_is_null = true;

    int _expires_in = 0;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = _access_token;
    rbind[0].buffer_length = ALEXA_TOKEN_MAXSIZE;
    rbind[0].length = &access_token_size;
    rbind[0].is_null = &access_token_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = _refresh_token;
    rbind[1].buffer_length = ALEXA_TOKEN_MAXSIZE;
    rbind[1].length = &refresh_token_size;
    rbind[1].is_null = &refresh_token_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&_expires_in;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = _region;
    rbind[3].buffer_length = ALEXA_REGION_MAXSIZE;
    rbind[3].length = &region_size;
    rbind[3].is_null = &region_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(_access_token, sizeof(_access_token),
                                  access_token_size, access_token_is_null);

        dba->set_terminating_byte(_refresh_token, sizeof(_refresh_token),
                                  refresh_token_size, refresh_token_is_null);

        dba->set_terminating_byte(_region, sizeof(_region), region_size,
                                  region_is_null);

        if (access_token) {
          *access_token = _access_token;
        }

        if (refresh_token) {
          *refresh_token = _refresh_token;
        }

        if (expires_in) {
          *expires_in = _expires_in;
        }

        if (region) {
          *region = _region;
        }

        result = true;
      }
    }
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

void supla_amazon_alexa_credentials_dao::set(int user_id,
                                             const string &access_token,
                                             const string &refresh_token,
                                             int expires_in) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_BIND pbind[4] = {};

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)access_token.c_str();
  pbind[0].buffer_length = access_token.size();

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)refresh_token.c_str();
  pbind[1].buffer_length = refresh_token.size();

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&expires_in;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&user_id;

  const char sql[] = "CALL `supla_update_amazon_alexa`(?,?,?,?)";

  MYSQL_STMT *stmt = NULL;
  dba->stmt_execute((void **)&stmt, sql, pbind, 4, true);

  if (stmt != NULL) {
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}

void supla_amazon_alexa_credentials_dao::remove(int user_id) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  const char sql[] = "CALL `supla_update_amazon_alexa`('','',0,?)";

  MYSQL_STMT *stmt = NULL;
  dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true);

  if (stmt != NULL) {
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}
