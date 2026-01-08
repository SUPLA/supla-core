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

#include "total_energy_logger_dao.h"

#include <mysql.h>

#include <pqxx/pqxx>

supla_total_energy_logger_dao::supla_total_energy_logger_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_cyclictask_dao(dba) {}

void supla_total_energy_logger_dao::set_longlong(unsigned _supla_int64_t *v,
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

void supla_total_energy_logger_dao::mariadb_add(
    int channel_id, TElectricityMeter_ExtendedValue_V3 *em_ev) {
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
  get_mdba()->stmt_execute((void **)&stmt, sql, pbind, 15, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_total_energy_logger_dao::tsdb_add(
    int channel_id, TElectricityMeter_ExtendedValue_V3 *em_ev) {
  pqxx::nontransaction ntx(*get_tsdba()->get_conn());

  bool not_zero = false;

  for (int a = 0; a < 3; a++) {
    if (em_ev->total_forward_active_energy[a] ||
        em_ev->total_reverse_active_energy[a] ||
        em_ev->total_forward_reactive_energy[a] ||
        em_ev->total_reverse_reactive_energy[a]) {
      not_zero = true;
      break;
    }
  }

  if (!not_zero && !em_ev->total_forward_active_energy_balanced &&
      !em_ev->total_reverse_active_energy_balanced) {
    return;
  }

  ntx.exec_params(
      "SELECT "
      "supla_add_em_log_item($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$"
      "15)",
      channel_id, em_ev->total_forward_active_energy[0],
      em_ev->total_reverse_active_energy[0],
      em_ev->total_forward_reactive_energy[0],
      em_ev->total_reverse_reactive_energy[0],
      em_ev->total_forward_active_energy[1],
      em_ev->total_reverse_active_energy[1],
      em_ev->total_forward_reactive_energy[1],
      em_ev->total_reverse_reactive_energy[1],
      em_ev->total_forward_active_energy[2],
      em_ev->total_reverse_active_energy[2],
      em_ev->total_forward_reactive_energy[2],
      em_ev->total_reverse_reactive_energy[2],
      em_ev->total_forward_active_energy_balanced,
      em_ev->total_reverse_active_energy_balanced);
}

void supla_total_energy_logger_dao::add(
    int channel_id, TElectricityMeter_ExtendedValue_V3 *em_ev) {
  if (!em_ev) {
    return;
  }

  if (get_mdba()) {
    mariadb_add(channel_id, em_ev);
  } else if (get_tsdba()) {
    try {
      tsdb_add(channel_id, em_ev);
    } catch (const std::exception &e) {
      get_tsdba()->log_exception(e);
    }
  }
}
