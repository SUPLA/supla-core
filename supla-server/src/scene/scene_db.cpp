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

#include "scene/scene_db.h"

#include <mysql.h>
#include <stdlib.h>
#include <string.h>

#include "actions/action_config.h"
#include "log.h"

supla_scene_db::supla_scene_db(void) : svrdb() {}

supla_scene_db::~supla_scene_db(void) {}

supla_scene_operation *supla_scene_db::convert(_supla_db_operation_row_t *row) {
  if (!row) {
    return NULL;
  }
  supla_scene_operation *op = new supla_scene_operation();
  if (op) {
    supla_action_config config;
    if (row->channel_id) {
      config.set_subject_id(row->channel_id);
      config.set_subject_type(stChannel);
    } else if (row->channel_group_id) {
      config.set_subject_id(row->channel_group_id);
      config.set_subject_type(stChannelGroup);
    } else if (row->scene_id) {
      config.set_subject_id(row->scene_id);
      config.set_subject_type(stScene);
    }

    config.set_action_id(row->action_id);
    config.apply_json_params(row->action_param);
    op->set_delay_ms(row->delay_ms);
    op->set_action_config(config);
  }
  return op;
}

std::list<supla_scene_operation *> supla_scene_db::get_operations(
    int scene_id) {
  std::list<supla_scene_operation *> result;
  _supla_db_operation_row_t row = {};

  unsigned long action_param_len = 0;
  my_bool action_param_is_null = true;

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT o.`owning_scene_id`, o.`channel_id`, o.`channel_group_id`, "
      "o.`scene_id`, o.`action`, o.`action_param`, o.`delay_ms` FROM "
      "`supla_scene_operation` o LEFT JOIN `supla_scene` s ON s.`id` = "
      "o.`owning_scene_id` WHERE s.`id` = ? AND s.`enabled` = 1 ORDER BY "
      "o.`owning_scene_id`, o.`id`";

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&scene_id;

  if (stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[9];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&row.owning_scene_id;
    rbind[0].buffer_length = sizeof(int);

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&row.channel_id;
    rbind[1].buffer_length = sizeof(int);

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&row.channel_group_id;
    rbind[2].buffer_length = sizeof(int);

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&row.scene_id;
    rbind[3].buffer_length = sizeof(int);

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&row.action_id;
    rbind[4].buffer_length = sizeof(int);

    rbind[5].buffer_type = MYSQL_TYPE_STRING;
    rbind[5].buffer = row.action_param;
    rbind[5].buffer_length = sizeof(row.action_param);
    rbind[5].length = &action_param_len;
    rbind[5].is_null = &action_param_is_null;

    rbind[6].buffer_type = MYSQL_TYPE_LONG;
    rbind[6].buffer = (char *)&row.delay_ms;
    rbind[6].buffer_length = sizeof(int);

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          set_terminating_byte(row.action_param, sizeof(row.action_param),
                               action_param_len, action_param_is_null);

          supla_scene_operation *op = convert(&row);
          if (op) {
            result.push_back(op);
          }
          row = {};
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}
