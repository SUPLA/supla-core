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

#include "client/client_scene_dao.h"

#include <mysql.h>

#include <list>

#include "log.h"
#include "proto.h"

typedef struct {
  int scene_id;
  char caption[SUPLA_SCENE_CAPTION_MAXSIZE];
  int user_icon_id;
  unsigned long caption_len;
  my_bool caption_is_null;
} _supla_scene_row_t;

supla_client_scene_dao::supla_client_scene_dao()
    : supla_abstract_client_scene_dao(), svrdb() {}

supla_client_scene_dao::~supla_client_scene_dao() {}

std::list<supla_client_scene *> supla_client_scene_dao::get_all_scenes(
    int user_id, int client_id) {
  std::list<supla_client_scene *> result;

  if (!connect()) {
    return result;
  }

  _supla_scene_row_t row = {};

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT s.id, s.caption, s.user_icon_id FROM `supla_scene` s LEFT JOIN "
      "supla_rel_aidloc al ON al.location_id = s.location_id LEFT JOIN "
      "supla_accessid a ON a.id = al.access_id LEFT JOIN supla_client c ON "
      "c.access_id = al.access_id WHERE s.user_id = ? AND s.enabled = 1 AND "
      "c.id = ? GROUP BY s.id";

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&client_id;

  if (stmt_execute((void **)&stmt, sql, pbind, 2, true)) {
    MYSQL_BIND rbind[3] = {};

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&row.scene_id;
    rbind[0].buffer_length = sizeof(int);

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = row.caption;
    rbind[1].buffer_length = sizeof(row.caption);
    rbind[1].length = &row.caption_len;
    rbind[1].is_null = &row.caption_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&row.user_icon_id;
    rbind[2].buffer_length = sizeof(int);

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          set_terminating_byte(row.caption, sizeof(row.caption),
                               row.caption_len, row.caption_is_null);

          supla_client_scene *scene = new supla_client_scene(row.scene_id);
          if (scene) {
            result.push_back(scene);
            row = {};
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  disconnect();

  return result;
}
