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

#include "log.h"

using std::string;

supla_abstract_electricity_logger_dao::supla_abstract_electricity_logger_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_cyclictask_dao(dba) {}

supla_abstract_electricity_logger_dao::
    ~supla_abstract_electricity_logger_dao() {}

void supla_abstract_electricity_logger_dao::add(const time_t &time,
                                                int channel_id, char phase,
                                                supla_simple_statiscics *stat,
                                                const string &procedure,
                                                unsigned char precision) {
  if (!stat || !stat->get_sample_count() || procedure.empty() || !phase) {
    return;
  }

  MYSQL_BIND pbind[6] = {};

  char format[10] = {};
  snprintf(format, sizeof(format), "%%0.%if", precision);

  char min[20] = {};
  snprintf(min, sizeof(min), format, stat->get_min());

  char max[20] = {};
  snprintf(max, sizeof(max), format, stat->get_max());

  char avg[20] = {};
  snprintf(avg, sizeof(avg), format, stat->get_avg());

  MYSQL_TIME mytime = get_mdba()->time_t_to_mytime(&time);

  pbind[0].buffer_type = MYSQL_TYPE_DATETIME;
  pbind[0].buffer = (char *)&mytime;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&channel_id;

  pbind[2].buffer_type = MYSQL_TYPE_TINY;
  pbind[2].buffer = (char *)&phase;

  pbind[3].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[3].buffer = min;
  pbind[3].buffer_length = strnlen(min, sizeof(min));

  pbind[4].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[4].buffer = max;
  pbind[4].buffer_length = strnlen(max, sizeof(max));

  pbind[5].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[5].buffer = avg;
  pbind[5].buffer_length = strnlen(avg, sizeof(avg));

  string sql = "CALL `";
  sql.append(procedure);
  sql.append("`(?,?,?,?,?,?)");

  MYSQL_STMT *stmt = nullptr;
  get_mdba()->stmt_execute((void **)&stmt, sql.c_str(), pbind, 6, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}
