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

#include "scene/scene_operations_dao.h"

#include <mysql.h>
#include <string.h>

#include <list>

#include "log.h"

#define SCENE_ACTION_PARAM_MAXSIZE 256

typedef struct {
  int channel_id;
  int channel_group_id;
  int scene_id;
  int action_id;
  char action_param[SCENE_ACTION_PARAM_MAXSIZE];
  unsigned long action_param_len;
  my_bool action_param_is_null;
  int delay_ms;
} _supla_scene_operation_row_t;

supla_scene_operations_dao::supla_scene_operations_dao()
    : supla_abstract_scene_operations_dao(), svrdb() {}

supla_scene_operations_dao::~supla_scene_operations_dao() {}

supla_scene_operations *supla_scene_operations_dao::get_scene_operations(
    int scene_id) {
  if (!connect()) {
    return NULL;
  }

  supla_scene_operations *operations = new supla_scene_operations();
  if (!operations) {
    disconnect();
    return NULL;
  }

  _supla_scene_operation_row_t row = {};

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT o.`channel_id`, o.`channel_group_id`, "
      "o.`scene_id`, o.`action`, o.`action_param`, o.`delay_ms` FROM "
      "`supla_scene_operation` o LEFT JOIN `supla_scene` s ON s.`id` = "
      "o.`owning_scene_id` WHERE s.`id` = ? AND s.`enabled` = 1 ORDER BY "
      "o.`id` ASC";

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&scene_id;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[6];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&row.channel_id;
    rbind[0].buffer_length = sizeof(int);

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&row.channel_group_id;
    rbind[1].buffer_length = sizeof(int);

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&row.scene_id;
    rbind[2].buffer_length = sizeof(int);

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&row.action_id;
    rbind[3].buffer_length = sizeof(int);

    rbind[4].buffer_type = MYSQL_TYPE_STRING;
    rbind[4].buffer = row.action_param;
    rbind[4].buffer_length = sizeof(row.action_param);
    rbind[4].length = &row.action_param_len;
    rbind[4].is_null = &row.action_param_is_null;

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&row.delay_ms;
    rbind[5].buffer_length = sizeof(int);

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          set_terminating_byte(row.action_param, sizeof(row.action_param),
                               row.action_param_len, row.action_param_is_null);

          supla_scene_operation *op = new supla_scene_operation();
          if (op) {
            supla_action_config config;
            if (row.channel_id) {
              config.set_subject_id(row.channel_id);
              config.set_subject_type(stChannel);
            } else if (row.channel_group_id) {
              config.set_subject_id(row.channel_group_id);
              config.set_subject_type(stChannelGroup);
            } else if (row.scene_id) {
              config.set_subject_id(row.scene_id);
              config.set_subject_type(stScene);
            }

            config.set_action_id(row.action_id);
            config.apply_json_params(row.action_param);
            op->set_delay_ms(row.delay_ms);
            op->set_action_config(config);

            operations->push(op);
            row = {};
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  disconnect();

  return operations;
}
