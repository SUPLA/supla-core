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

#include "pn_recipient_dao.h"

#include <mysql.h>

#include "log.h"

supla_pn_recipient_dao::supla_pn_recipient_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_pn_recipient_dao::~supla_pn_recipient_dao(void) {}

void supla_pn_recipient_dao::remove(int user_id,
                                    supla_pn_recipient *recipient) {
  if (!user_id || !recipient || !recipient->get_client_id()) {
    return;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_BIND pbind[2] = {};

  int client_id = recipient->get_client_id();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&client_id;

  const char sql[] = "CALL `supla_remove_push_recipients`(?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, &pbind, 2, true);

  if (stmt != nullptr) {
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}

void supla_pn_recipient_dao::get_recipients(int user_id,
                                            int push_notification_id,
                                            supla_pn_recipients *recipients) {
  recipients->clear();

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT c.push_token, c.id, c.platform, c.app_id, c.devel_env FROM "
      "supla_client c, supla_rel_aid_pushnotification p WHERE "
      "p.push_notification_id = ? AND c.user_id = ? AND c.access_id = "
      "p.access_id AND c.push_token IS NOT NULL AND (c.platform = 1 OR "
      "c.platform = 2)";

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&push_notification_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&user_id;

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 2, true)) {
    MYSQL_BIND rbind[5] = {};

    char token[256] = {};
    int client_id = 0;
    char platform = 0;
    int app_id = 0;
    char devel_env = 0;

    unsigned long token_len = 0;
    my_bool token_is_null = false;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = token;
    rbind[0].buffer_length = sizeof(token);
    rbind[0].length = &token_len;
    rbind[0].is_null = &token_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&client_id;
    rbind[1].buffer_length = sizeof(client_id);

    rbind[2].buffer_type = MYSQL_TYPE_TINY;
    rbind[2].buffer = &platform;
    rbind[2].buffer_length = sizeof(platform);

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&app_id;
    rbind[3].buffer_length = sizeof(app_id);

    rbind[4].buffer_type = MYSQL_TYPE_TINY;
    rbind[4].buffer = &devel_env;
    rbind[4].buffer_length = sizeof(devel_env);

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          dba->set_terminating_byte(token, sizeof(token), token_len,
                                    token_is_null);

          supla_pn_recipient *recipient =
              new supla_pn_recipient(client_id, app_id, devel_env > 0, token);
          if (recipient) {
            recipients->add(recipient, static_cast<_platform_e>(platform));
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}
