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

#include <my_global.h>
#include <mysql.h>
#include <string.h>

#include "database.h"
#include "log.h"
#include "safearray.h"
#include "schedulercfg.h"
#include "tools.h"

char *database::cfg_get_host(void) {
  return scfg_string(CFG_SCHEDULER_MYSQL_HOST) == NULL
             ? scfg_string(CFG_MYSQL_HOST)
             : scfg_string(CFG_SCHEDULER_MYSQL_HOST);
}

char *database::cfg_get_user(void) {
  return scfg_string(CFG_SCHEDULER_MYSQL_HOST) == NULL
             ? scfg_string(CFG_MYSQL_USER)
             : scfg_string(CFG_SCHEDULER_MYSQL_USER);
}
char *database::cfg_get_password(void) {
  return scfg_string(CFG_SCHEDULER_MYSQL_HOST) == NULL
             ? scfg_string(CFG_MYSQL_PASSWORD)
             : scfg_string(CFG_SCHEDULER_MYSQL_PASSWORD);
}
char *database::cfg_get_database(void) {
  return scfg_string(CFG_SCHEDULER_MYSQL_HOST) == NULL
             ? scfg_string(CFG_MYSQL_DB)
             : scfg_string(CFG_SCHEDULER_MYSQL_DB);
}
int database::cfg_get_port(void) {
  return scfg_string(CFG_SCHEDULER_MYSQL_HOST) == NULL
             ? scfg_int(CFG_MYSQL_PORT)
             : scfg_int(CFG_SCHEDULER_MYSQL_PORT);
}

void database::get_s_executions(void *s_exec_arr, int limit) {
  MYSQL_STMT *stmt;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  if (s_exec_arr == NULL) return;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&limit;

  if (stmt_execute((void **)&stmt,
                   "SELECT e.`id`, e.`schedule_id`, s.`user_id`, "
                   "c.`iodevice_id`, s.`channel_id`, c.`func`, c.`param1`, "
                   "c.`param2`, c.`param3`, s.`action`, s.`action_param`, "
                   "UNIX_TIMESTAMP(e.`planned_timestamp`), "
                   "UNIX_TIMESTAMP(e.`retry_timestamp`), e.`retry_count`, "
                   "s.`retry` FROM `supla_scheduled_executions` AS e, "
                   "`supla_schedule` AS s, `supla_dev_channel` AS c WHERE "
                   "e.`schedule_id` = s.`id` AND s.`channel_id` = c.`id` AND "
                   "e.`result_timestamp` IS NULL AND e.`fetched_timestamp` IS "
                   "NULL AND ( (e.`retry_timestamp` IS NULL AND "
                   "e.`planned_timestamp` <= UTC_TIMESTAMP()) OR "
                   "(e.`retry_timestamp` IS NOT NULL AND e.`retry_timestamp` "
                   "<= UTC_TIMESTAMP())) LIMIT ?",
                   pbind, 1, true)) {
    my_bool is_null[4];

    MYSQL_BIND rbind[15];
    memset(rbind, 0, sizeof(rbind));

    int id, schedule_id, user_id, device_id, channel_id, channel_func,
        channel_param1, channel_param2, channel_param3;
    int action, planned_timestamp, retry_timestamp, retry_count,
        retry_when_fail;

    unsigned long length;
    char action_param[256];

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&id;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&schedule_id;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&user_id;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&device_id;

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&channel_id;

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&channel_func;

    rbind[6].buffer_type = MYSQL_TYPE_LONG;
    rbind[6].buffer = (char *)&channel_param1;

    rbind[7].buffer_type = MYSQL_TYPE_LONG;
    rbind[7].buffer = (char *)&channel_param2;

    rbind[8].buffer_type = MYSQL_TYPE_LONG;
    rbind[8].buffer = (char *)&channel_param3;

    rbind[9].buffer_type = MYSQL_TYPE_LONG;
    rbind[9].buffer = (char *)&action;

    rbind[10].buffer_type = MYSQL_TYPE_STRING;
    rbind[10].buffer = action_param;
    rbind[10].is_null = &is_null[0];
    rbind[10].buffer_length = 256;
    rbind[10].length = &length;

    rbind[11].buffer_type = MYSQL_TYPE_LONG;
    rbind[11].buffer = &planned_timestamp;

    rbind[12].buffer_type = MYSQL_TYPE_LONG;
    rbind[12].buffer = &retry_timestamp;
    rbind[12].is_null = &is_null[1];

    rbind[13].buffer_type = MYSQL_TYPE_LONG;
    rbind[13].buffer = &retry_count;
    rbind[13].is_null = &is_null[2];

    rbind[14].buffer_type = MYSQL_TYPE_LONG;
    rbind[14].buffer = &retry_when_fail;
    rbind[14].is_null = &is_null[3];

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          s_exec_t *s_exec = (s_exec_t *)malloc(sizeof(s_exec_t));

          if (s_exec != NULL) {
            memset(s_exec, 0, sizeof(s_exec_t));

            s_exec->id = id;
            s_exec->schedule_id = schedule_id;
            s_exec->user_id = user_id;
            s_exec->iodevice_id = device_id;
            s_exec->channel_id = channel_id;
            s_exec->channel_func = channel_func;
            s_exec->channel_param1 = channel_param1;
            s_exec->channel_param2 = channel_param2;
            s_exec->channel_param3 = channel_param3;
            s_exec->action = action;
            s_exec->planned_timestamp = planned_timestamp;

            if (is_null[0] == false) {
              action_param[255] = 0;
              s_exec->action_param = strdup(action_param);
            }

            s_exec->retry_timestamp = is_null[1] ? 0 : retry_timestamp;
            s_exec->retry_count = is_null[2] ? 0 : retry_count;
            s_exec->retry_when_fail = !is_null[3] && retry_when_fail > 0;

            if (safe_array_add(s_exec_arr, s_exec) == -1) {
              if (s_exec->action_param != NULL) {
                free(s_exec->action_param);
              }

              free(s_exec);
            }
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }
}

bool database::get_channel(supla_channel *channel) {
  if (channel == NULL || channel->id == 0) return false;

  MYSQL_STMT *stmt;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  bool result = false;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel->id;

  if (stmt_execute((void **)&stmt,
                   "SELECT  `type`, `func`, `param1`, `param2`, `param3`, "
                   "`iodevice_id` FROM `supla_dev_channel`  WHERE `id` = ?",
                   pbind, 1, true)) {
    MYSQL_BIND rbind[6];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&channel->type;

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&channel->func;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&channel->param1;

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&channel->param2;

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&channel->param3;

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&channel->iodevice_id;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt)) {
        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}

void database::set_expired_result(int expired_time) {
  MYSQL_STMT *stmt;
  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  int result = ACTION_EXECUTION_RESULT_EXPIRED;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&result;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&expired_time;

  if (stmt_execute((void **)&stmt,
                   "UPDATE `supla_scheduled_executions` SET "
                   "`result_timestamp`= UTC_TIMESTAMP(), `result` = ? WHERE "
                   "result IS NULL AND fetched_timestamp IS NULL AND "
                   "planned_timestamp + INTERVAL ? SECOND <= UTC_TIMESTAMP()",
                   pbind, 2, true)) {
    mysql_stmt_close(stmt);
  }
}

void database::set_zombie_result(int zombie_time) {
  MYSQL_STMT *stmt;
  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  int result = ACTION_EXECUTION_RESULT_ZOMBIE;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&result;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&zombie_time;

  if (stmt_execute((void **)&stmt,
                   "UPDATE `supla_scheduled_executions` SET "
                   "`result_timestamp`= UTC_TIMESTAMP(), `result` = ? WHERE "
                   "result IS NULL AND fetched_timestamp IS NOT NULL AND "
                   "fetched_timestamp + INTERVAL ? SECOND <= UTC_TIMESTAMP()",
                   pbind, 2, true)) {
    mysql_stmt_close(stmt);
  }
}

bool database::set_fetched(int id) {
  bool result = false;
  MYSQL_STMT *stmt;
  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&id;

  if (stmt_execute((void **)&stmt,
                   "UPDATE `supla_scheduled_executions` SET `consumed` = 1, "
                   "`fetched_timestamp`= UTC_TIMESTAMP() WHERE `id` = ? AND "
                   "`fetched_timestamp` IS NULL",
                   pbind, 1, true)) {
    result = mysql_stmt_affected_rows(stmt) == 1;
    mysql_stmt_close(stmt);
  }

  return result;
}

void database::set_unfetched(int id) {
  MYSQL_STMT *stmt;
  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&id;

  if (stmt_execute((void **)&stmt,
                   "UPDATE `supla_scheduled_executions` SET "
                   "`fetched_timestamp`= NULL WHERE `id` = ?",
                   pbind, 1, true)) {
    mysql_stmt_close(stmt);
  }
}

bool database::set_retry(int id, int sec) {
  bool result = false;
  MYSQL_STMT *stmt;
  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&sec;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&id;

  if (stmt_execute((void **)&stmt,
                   "UPDATE `supla_scheduled_executions` SET "
                   "`fetched_timestamp`= NULL, `retry_timestamp`= "
                   "UTC_TIMESTAMP() + INTERVAL ? SECOND, `retry_count` = "
                   "IFNULL(`retry_count`, 0)+1 WHERE `id` = ? AND "
                   "`fetched_timestamp` IS NOT NULL",
                   pbind, 2, true)) {
    result = mysql_stmt_affected_rows(stmt) == 1;
    mysql_stmt_close(stmt);
  }

  return result;
}

bool database::set_result(int id, int result) {
  bool success = false;
  MYSQL_STMT *stmt;
  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&result;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&id;

  if (stmt_execute((void **)&stmt,
                   "UPDATE `supla_scheduled_executions` SET "
                   "`fetched_timestamp`= NULL, `result_timestamp`= "
                   "UTC_TIMESTAMP(), `result` = ? WHERE `id` = ? AND "
                   "`fetched_timestamp` IS NOT NULL",
                   pbind, 2, true)) {
    success = mysql_stmt_affected_rows(stmt) == 1;
    mysql_stmt_close(stmt);
  }

  return success;
}
