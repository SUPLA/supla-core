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

#include "scene/scene_dao.h"

#include <mysql.h>
#include <string.h>

#include "log.h"

supla_scene_dao::supla_scene_dao(supla_abstract_db_access_provider *dba)
    : supla_abstract_scene_dao() {
  this->dba = dba;
}

supla_scene_dao::~supla_scene_dao() {}

unsigned int supla_scene_dao::get_estimated_execution_time(int scene_id) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return 0;
  }

  unsigned int result = 0;

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT estimated_execution_time FROM supla_scene WHERE id = ?";

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&scene_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind = {};

    rbind.buffer_type = MYSQL_TYPE_LONG;
    rbind.buffer = (char *)&result;
    rbind.buffer_length = sizeof(int);

    if (mysql_stmt_bind_result(stmt, &rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) != 1 || mysql_stmt_fetch(stmt)) {
        result = 0;
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}
