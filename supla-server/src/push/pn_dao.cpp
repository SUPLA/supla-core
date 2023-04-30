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

#include "pn_dao.h"

#include <mysql.h>

#include "log.h"

supla_pn_dao::supla_pn_dao(supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_pn_dao::~supla_pn_dao(void) {}

bool supla_pn_dao::get(int user_id, supla_push_notification *push) {
  if (!user_id || !push || !push->get_id()) {
    return false;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  MYSQL_BIND pbind[2] = {};

  int id = push->get_id();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&user_id;

  const char sql[] =
      "SELECT title, body, IFNULL(sound, 0) FROM `supla_push_notification` "
      "WHERE id = ? AND user_id = ?";

  MYSQL_STMT *stmt = nullptr;
  bool result = false;

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 2, true)) {
    MYSQL_BIND rbind[3] = {};

    char title[100] = {};
    char body[256] = {};

    unsigned long title_len = 0;
    unsigned long body_len = 0;
    my_bool title_is_null = false;
    my_bool body_is_null = false;
    int sound = 0;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = title;
    rbind[0].buffer_length = sizeof(title);
    rbind[0].length = &title_len;
    rbind[0].is_null = &title_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = body;
    rbind[1].buffer_length = sizeof(body);
    rbind[1].length = &body_len;
    rbind[1].is_null = &body_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&sound;
    rbind[2].buffer_length = sizeof(int);

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        dba->set_terminating_byte(title, sizeof(title), title_len,
                                  title_is_null);

        dba->set_terminating_byte(body, sizeof(body), body_len, body_is_null);

        push->set_title(title);
        push->set_body(body);
        push->set_sound(sound);
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
