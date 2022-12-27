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

#include "datalogger/electricity_logger_dao.h"

#include <mysql.h>

supla_electricity_logger_dao::supla_electricity_logger_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

void supla_electricity_logger_dao::set_longlong(unsigned _supla_int64_t *v,
                                                void *pbind,
                                                bool *not_null_flag) {
  if (*v == 0) {
    ((MYSQL_BIND *)pbind)->buffer_type = MYSQL_TYPE_NULL;
  } else {
    ((MYSQL_BIND *)pbind)->buffer_type = MYSQL_TYPE_LONGLONG;
    ((MYSQL_BIND *)pbind)->buffer = (char *)v;
    if (not_null_flag) {
      *not_null_flag = true;
    }
  }
}

void supla_electricity_logger_dao::add(
    int channel_id, TElectricityMeter_ExtendedValue_V2 *em_ev) {
  if (!em_ev) {
    return;
  }

  MYSQL_BIND pbind[15] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  int n = 0;
  bool not_null = false;
  for (int a = 0; a < 3; a++) {
    set_longlong(&em_ev->total_forward_active_energy[a], &pbind[1 + n],
                 &not_null);
    set_longlong(&em_ev->total_reverse_active_energy[a], &pbind[2 + n],
                 &not_null);
    set_longlong(&em_ev->total_forward_reactive_energy[a], &pbind[3 + n],
                 &not_null);
    set_longlong(&em_ev->total_reverse_reactive_energy[a], &pbind[4 + n],
                 &not_null);

    n += 4;
  }

  set_longlong(&em_ev->total_forward_active_energy_balanced, &pbind[13],
               &not_null);
  set_longlong(&em_ev->total_reverse_active_energy_balanced, &pbind[14],
               &not_null);

  if (!not_null) {
    return;
  }

  const char sql[] =
      "CALL `supla_add_em_log_item`(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, pbind, 15, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}
