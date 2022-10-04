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

#include "datalogger/thermostat_logger_dao.h"

#include <mysql.h>
#include <stdio.h>
#include <string.h>

supla_thermostat_logger_dao::supla_thermostat_logger_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

void supla_thermostat_logger_dao::add(
    supla_channel_thermostat_measurement *th) {
  if (!th) {
    return;
  }

  char buff1[20] = {};
  char buff2[20] = {};
  MYSQL_BIND pbind[4] = {};

  int ChannelId = th->getChannelId();
  snprintf(buff1, sizeof(buff1), "%05.2f", th->getMeasuredTemperature());
  snprintf(buff2, sizeof(buff2), "%05.2f", th->getPresetTemperature());

  char on = th->getOn() ? 1 : 0;

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ChannelId;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = (char *)buff1;
  pbind[1].buffer_length = strnlen(buff1, 20);

  pbind[2].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[2].buffer = (char *)buff2;
  pbind[2].buffer_length = strnlen(buff2, 20);

  pbind[3].buffer_type = MYSQL_TYPE_TINY;
  pbind[3].buffer = (char *)&on;
  pbind[3].buffer_length = sizeof(char);

  const char sql[] = "CALL `supla_add_thermostat_log_item`(?,?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, pbind, 4, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}
