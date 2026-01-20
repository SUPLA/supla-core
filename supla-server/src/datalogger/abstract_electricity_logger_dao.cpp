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

#include "abstract_electricity_logger_dao.h"

#include <mysql.h>
#include <string.h>

#include <cstdio>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "log.h"

using std::string;

#define BUFF_SIZE 20

supla_abstract_electricity_logger_dao::supla_abstract_electricity_logger_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_cyclictask_dao(dba) {}

supla_abstract_electricity_logger_dao::
    ~supla_abstract_electricity_logger_dao() {}

void supla_abstract_electricity_logger_dao ::mariadb_add(
    const time_t &time, int channel_id, char phase, const string &procedure,
    char *min, char *max, char *avg) {
  MYSQL_BIND pbind[6] = {};

  MYSQL_TIME mytime = get_mdba()->time_t_to_mytime(&time);

  pbind[0].buffer_type = MYSQL_TYPE_DATETIME;
  pbind[0].buffer = (char *)&mytime;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&channel_id;

  pbind[2].buffer_type = MYSQL_TYPE_TINY;
  pbind[2].buffer = (char *)&phase;

  pbind[3].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[3].buffer = min;
  pbind[3].buffer_length = strnlen(min, BUFF_SIZE);

  pbind[4].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[4].buffer = max;
  pbind[4].buffer_length = strnlen(max, BUFF_SIZE);

  pbind[5].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[5].buffer = avg;
  pbind[5].buffer_length = strnlen(avg, BUFF_SIZE);

  string sql = "CALL `";
  sql.append(procedure);
  sql.append("`(?,?,?,?,?,?)");

  MYSQL_STMT *stmt = nullptr;
  get_mdba()->stmt_execute((void **)&stmt, sql.c_str(), pbind, 6, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_abstract_electricity_logger_dao ::tsdb_add(
    const time_t &time, int channel_id, short phase, const string &procedure,
    char *min, char *max, char *avg) {
  pqxx::nontransaction ntx(*get_tsdba()->get_conn());

  string sql = "SELECT ";
  sql.append(procedure);
  sql.append("($1,$2,$3,$4,$5,$6)");

  string time_str = get_tsdba()->time_to_timestamp_string(time);

  ntx.exec_params(sql, time_str, channel_id, phase, min, max, avg);
}

void supla_abstract_electricity_logger_dao::add(const time_t &time,
                                                int channel_id, char phase,
                                                supla_simple_statiscics *stat,
                                                const string &procedure,
                                                unsigned char precision) {
  if (!stat || !stat->get_sample_count() || procedure.empty() || !phase) {
    return;
  }

  char format[10] = {};
  snprintf(format, sizeof(format), "%%0.%if", precision);

  char min[BUFF_SIZE] = {};
  snprintf(min, BUFF_SIZE, format, stat->get_min());

  char max[BUFF_SIZE] = {};
  snprintf(max, BUFF_SIZE, format, stat->get_max());

  char avg[BUFF_SIZE] = {};
  snprintf(avg, BUFF_SIZE, format, stat->get_avg());

  if (get_mdba()) {
    mariadb_add(time, channel_id, phase, procedure, min, max, avg);
  } else if (get_tsdba()) {
    try {
      tsdb_add(time, channel_id, phase, procedure, min, max, avg);
    } catch (const std::exception &e) {
      get_tsdba()->log_exception(e, channel_id);
    }
  }
}
