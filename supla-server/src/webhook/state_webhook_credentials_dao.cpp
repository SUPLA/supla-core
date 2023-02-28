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

#include "webhook/state_webhook_credentials_dao.h"

#include <mysql.h>

#include "log.h"

using std::string;

#define WEBHOOK_TOKEN_MAXSIZE 255
#define WEBHOOK_URL_MAXSIZE 255
#define WEBHOOK_FUNCTIONS_IDS_MAXSIZE 255

supla_state_webhook_credentials_dao::supla_state_webhook_credentials_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_state_webhook_credentials_dao::~supla_state_webhook_credentials_dao(
    void) {}

void supla_state_webhook_credentials_dao::get(int user_id, string *access_token,
                                              string *refresh_token,
                                              int *expires_in, string *url,
                                              string *function_ids) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  char sql[] =
      "SELECT `access_token`, `refresh_token`, TIMESTAMPDIFF(SECOND, "
      "UTC_TIMESTAMP(), expires_at) `expires_in`, `url`, `functions_ids` "
      "FROM "
      "`supla_state_webhooks` WHERE user_id = ? AND enabled = 1 AND "
      "LENGTH(access_token) > 0";

  MYSQL_STMT *stmt = nullptr;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind[5] = {};

    char _access_token[WEBHOOK_TOKEN_MAXSIZE] = {};
    unsigned long access_token_size = 0;
    my_bool access_token_is_null = false;

    char _refresh_token[WEBHOOK_TOKEN_MAXSIZE] = {};
    unsigned long refresh_token_size = 0;
    my_bool refresh_token_is_null = false;

    char _url[WEBHOOK_URL_MAXSIZE] = {};
    unsigned long url_size = 0;
    my_bool url_is_null = false;

    char _function_ids[WEBHOOK_FUNCTIONS_IDS_MAXSIZE] = {};
    unsigned long function_ids_size = 0;
    my_bool function_ids_is_null = false;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = _access_token;
    rbind[0].buffer_length = sizeof(_access_token);
    rbind[0].length = &access_token_size;
    rbind[0].is_null = &access_token_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = _refresh_token;
    rbind[1].buffer_length = sizeof(_refresh_token);
    rbind[1].length = &refresh_token_size;
    rbind[1].is_null = &refresh_token_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)expires_in;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = _url;
    rbind[3].buffer_length = sizeof(_url);
    rbind[3].length = &url_size;
    rbind[3].is_null = &url_is_null;

    rbind[4].buffer_type = MYSQL_TYPE_STRING;
    rbind[4].buffer = _function_ids;
    rbind[4].buffer_length = sizeof(_function_ids);
    rbind[4].length = &function_ids_size;
    rbind[4].is_null = &function_ids_is_null;

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
        dba->set_terminating_byte(_url, sizeof(_url), url_size, url_is_null);
        dba->set_terminating_byte(_function_ids, sizeof(_function_ids),
                                  function_ids_size, function_ids_is_null);

        *access_token = _access_token;
        *refresh_token = _refresh_token;
        *url = _url;
        *function_ids = _function_ids;
      }
    }
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}

void supla_state_webhook_credentials_dao::set(int user_id,
                                              const string access_token,
                                              const string refresh_token,
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

  const char sql[] = "CALL `supla_update_state_webhook`(?,?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, pbind, 4, true);

  if (stmt != nullptr) {
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}

void supla_state_webhook_credentials_dao::remove(int user_id) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  const char sql[] = "CALL `supla_update_state_webhook`('','',0,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true);

  if (stmt != nullptr) {
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}
