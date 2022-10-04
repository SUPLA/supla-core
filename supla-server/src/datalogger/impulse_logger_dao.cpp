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

#include "datalogger/impulse_logger_dao.h"

#include <mysql.h>

supla_impulse_logger_dao::supla_impulse_logger_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

void supla_impulse_logger_dao::add(supla_channel_ic_measurement *ic) {
  if (!ic) {
    return;
  }

  MYSQL_BIND pbind[3] = {};

  int ChannelId = ic->getChannelId();
  unsigned _supla_int64_t counter = ic->getCounter();
  unsigned _supla_int64_t calculatedValue = ic->getCalculatedValue();

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ChannelId;
  pbind[1].buffer_type = MYSQL_TYPE_LONGLONG;
  pbind[1].buffer = (char *)&counter;
  pbind[2].buffer_type = MYSQL_TYPE_LONGLONG;
  pbind[2].buffer = (char *)&calculatedValue;

  const char sql[] = "CALL `supla_add_ic_log_item`(?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, pbind, 3, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}
