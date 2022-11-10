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

#include "cyclictasks/auto_gate_closing_dao.h"

#include <mysql.h>
#include <stdio.h>

#include "log.h"

using std::vector;

supla_auto_gate_closing_dao::supla_auto_gate_closing_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_auto_gate_closing_dao(dba) {}

supla_auto_gate_closing_dao::~supla_auto_gate_closing_dao() {}

vector<supla_abstract_auto_gate_closing_dao::item_t>
supla_auto_gate_closing_dao::get_all_active(void) {
  vector<supla_abstract_auto_gate_closing_dao::item_t> result;

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT user_id, device_id, channel_id, IFNULL(TIMESTAMPDIFF(SECOND, "
      "closing_attempt, UTC_TIMESTAMP()), ?) AS `ssla`, CASE WHEN "
      "last_seen_open IS NULL THEN 0 ELSE 1 END AS `so` FROM "
      "supla_v_auto_gate_closing WHERE enabled = 1 AND is_now_active = 1 ORDER "
      "BY user_id, device_id, channel_id";

  MYSQL_BIND pbind = {};

  int attempt_retry_time_sec = ATTEMPT_RETRY_TIME_SEC + 1;

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&attempt_retry_time_sec;

  if (get_dba()->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    supla_abstract_auto_gate_closing_dao::item_t i = {};

    MYSQL_BIND rbind[5] = {};
    char seen_open = 0;

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&i.user_id;
    rbind[0].buffer_length = sizeof(int);

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&i.device_id;
    rbind[1].buffer_length = sizeof(int);

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&i.channel_id;
    rbind[2].buffer_length = sizeof(int);

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&i.seconds_since_last_attempt;
    rbind[3].buffer_length = sizeof(int);

    rbind[4].buffer_type = MYSQL_TYPE_TINY;
    rbind[4].buffer = (char *)&seen_open;
    rbind[4].buffer_length = sizeof(char);

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          i.seen_open = seen_open == 1;
          result.push_back(i);
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}

int supla_auto_gate_closing_dao::mark_gate_open(int channel_id) {
  const char sql[] = "CALL `supla_mark_gate_open`(?)";

  return get_dba()->get_int(channel_id, 0, sql);
}

void supla_auto_gate_closing_dao::mark_gate_closed(int channel_id) {
  char sql[51];
  snprintf(sql, sizeof(sql), "CALL `supla_mark_gate_closed`(%i)", channel_id);

  get_dba()->query(sql, true);
}

void supla_auto_gate_closing_dao::set_closing_attempt(int channel_id) {
  char sql[51];
  snprintf(sql, sizeof(sql), "CALL `supla_set_closing_attempt`(%i)",
           channel_id);

  get_dba()->query(sql, true);
}
