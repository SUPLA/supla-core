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

#include "google/google_home_credentials_dao.h"

#include <mysql.h>

#include "log.h"

using std::string;

#define GH_TOKEN_MAXSIZE 255

supla_google_home_credentials_dao::supla_google_home_credentials_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_google_home_credentials_dao::~supla_google_home_credentials_dao(void) {}

string supla_google_home_credentials_dao::get_access_token(int user_id) {
  string result;

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return result;
  }

  char sql[] =
      "SELECT `access_token` FROM `supla_google_home` WHERE user_id = ? AND "
      "LENGTH(access_token) > 0";

  MYSQL_STMT *stmt = nullptr;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind = {};

    char access_token[GH_TOKEN_MAXSIZE + 1] = {};
    unsigned long access_token_size = 0;
    my_bool access_token_is_null = true;

    rbind.buffer_type = MYSQL_TYPE_STRING;
    rbind.buffer = access_token;
    rbind.buffer_length = GH_TOKEN_MAXSIZE;
    rbind.length = &access_token_size;
    rbind.is_null = &access_token_is_null;

    if (mysql_stmt_bind_result(stmt, &rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(access_token, sizeof(access_token),
                                  access_token_size, access_token_is_null);

        result = access_token;
      }
    }
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}
