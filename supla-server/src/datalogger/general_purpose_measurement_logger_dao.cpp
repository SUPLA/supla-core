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

#include "datalogger/general_purpose_measurement_logger_dao.h"

#include <string.h>

#include <pqxx/pqxx>

#include "log.h"

supla_general_purpose_measurement_logger_dao::
    supla_general_purpose_measurement_logger_dao(
        supla_abstract_db_access_provider *dba)
    : supla_abstract_cyclictask_dao(dba) {}

void supla_general_purpose_measurement_logger_dao::mariadb_add(
    int channel_id, double first, double last, double avg, double min,
    double max) {
  MYSQL_BIND pbind[6] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_DOUBLE;
  pbind[1].buffer = (char *)&first;

  pbind[2].buffer_type = MYSQL_TYPE_DOUBLE;
  pbind[2].buffer = (char *)&last;

  pbind[3].buffer_type = MYSQL_TYPE_DOUBLE;
  pbind[3].buffer = (char *)&avg;

  pbind[4].buffer_type = MYSQL_TYPE_DOUBLE;
  pbind[4].buffer = (char *)&max;

  pbind[5].buffer_type = MYSQL_TYPE_DOUBLE;
  pbind[5].buffer = (char *)&min;

  const char sql[] = "CALL `supla_add_gp_measurement_log_item`(?,?,?,?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  get_mdba()->stmt_execute((void **)&stmt, sql, pbind, 6, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_general_purpose_measurement_logger_dao::tsdb_add(
    int channel_id, double first, double last, double avg, double min,
    double max) {
  pqxx::nontransaction ntx(*get_tsdba()->get_conn());

  ntx.exec_params(
      "SELECT supla_add_gp_measurement_log_item ($1,$2,$3,$4,$5,$6)",
      channel_id, first, last, avg, max, min);
}

void supla_general_purpose_measurement_logger_dao::add(
    supla_general_purpose_measurement_analyzer *analyzer) {
  if (!analyzer) {
    return;
  }

  if (get_mdba()) {
    mariadb_add(analyzer->get_channel_id(), analyzer->get_first(),
                analyzer->get_last(), analyzer->get_time_weighted_avg(),
                analyzer->get_min(), analyzer->get_max());
  } else if (get_tsdba()) {
    try {
      tsdb_add(analyzer->get_channel_id(), analyzer->get_first(),
               analyzer->get_last(), analyzer->get_time_weighted_avg(),
               analyzer->get_min(), analyzer->get_max());
    } catch (const std::exception &e) {
      get_tsdba()->log_exception(e, analyzer->get_channel_id());
    }
  }
}
