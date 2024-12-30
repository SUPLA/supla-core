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

unsigned int supla_scene_dao::get_estimated_execution_time_and_active_period(
    int scene_id, supla_active_period *active_period) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return 0;
  }

  unsigned int result = 0;

  MYSQL_TIME active_from = {};
  my_bool active_from_is_null = false;

  MYSQL_TIME active_to = {};
  my_bool active_to_is_null = false;

  char active_hours[769] = {};
  unsigned long active_hours_len = 0;
  my_bool active_hours_are_null = false;

  char activity_conditions[1024] = {};
  unsigned long activity_conditions_len = 0;
  my_bool activity_conditions_are_null = false;

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT estimated_execution_time, active_from, active_to, active_hours, "
      "activity_conditions FROM supla_scene WHERE id = ?";

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&scene_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind[5] = {};

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&result;
    rbind[0].buffer_length = sizeof(int);

    rbind[1].buffer_type = MYSQL_TYPE_DATETIME;
    rbind[1].buffer = (char *)&active_from;
    rbind[1].buffer_length = sizeof(active_from);
    rbind[1].is_null = &active_from_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_DATETIME;
    rbind[2].buffer = (char *)&active_to;
    rbind[2].buffer_length = sizeof(active_to);
    rbind[2].is_null = &active_to_is_null;

    rbind[3].buffer_type = MYSQL_TYPE_STRING;
    rbind[3].buffer = active_hours;
    rbind[3].buffer_length = sizeof(active_hours);
    rbind[3].length = &active_hours_len;
    rbind[3].is_null = &active_hours_are_null;

    rbind[4].buffer_type = MYSQL_TYPE_STRING;
    rbind[4].buffer = activity_conditions;
    rbind[4].buffer_length = sizeof(activity_conditions);
    rbind[4].length = &activity_conditions_len;
    rbind[4].is_null = &activity_conditions_are_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) != 1 || mysql_stmt_fetch(stmt)) {
        result = 0;
      } else if (active_period) {
        unsigned long _active_from =
            active_from_is_null ? 0 : dba->mytime_to_time_t(&active_from);
        unsigned long _active_to =
            active_to_is_null ? 0 : dba->mytime_to_time_t(&active_to);

        *active_period = supla_active_period(_active_from, _active_to,
                                             active_hours, activity_conditions);
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}
