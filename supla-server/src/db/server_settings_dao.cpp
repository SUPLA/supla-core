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

#include "server_settings_dao.h"

#include <mysql.h>

#include "log.h"

using std::string;

supla_server_settins_dao::supla_server_settins_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_server_settins_dao::~supla_server_settins_dao(void) {}

string supla_server_settins_dao::get_target_token(void) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return "";
  }

  string result;

  char sql[] =
      "SELECT value FROM supla_settings_string WHERE name = 'target_token' "
      "LIMIT 1";

  MYSQL_STMT *stmt = nullptr;

  if (dba->stmt_execute((void **)&stmt, sql, nullptr, 0, true)) {
    MYSQL_BIND rbind = {};

    char token[1025] = {};
    unsigned long token_len = 0;
    my_bool token_is_null = false;

    rbind.buffer_type = MYSQL_TYPE_STRING;
    rbind.buffer = token;
    rbind.buffer_length = sizeof(token);
    rbind.length = &token_len;
    rbind.is_null = &token_is_null;

    if (mysql_stmt_bind_result(stmt, &rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(token, sizeof(token), token_len,
                                  token_is_null);

        result = token;
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}
