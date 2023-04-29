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

#include "schedule/schedule_dao.h"

#include <mysql.h>

supla_schedule_dao::supla_schedule_dao(supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_schedule_dao::~supla_schedule_dao() {}

void supla_schedule_dao::enable(int user_id, int schedule_id, bool enable) {
  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return;
  }

  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&schedule_id;

  const char sql_enable[] = "CALL `supla_enable_schedule`(?,?)";
  const char sql_disable[] = "CALL `supla_disable_schedule`(?,?)";

  MYSQL_STMT *stmt = nullptr;

  dba->stmt_execute((void **)&stmt, enable ? sql_enable : sql_disable, pbind, 2,
                    true);

  if (stmt != nullptr) {
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }
}
