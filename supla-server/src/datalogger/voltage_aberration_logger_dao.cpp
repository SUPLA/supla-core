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

#include "voltage_aberration_logger_dao.h"

#include <string.h>

#include <cstdio>
#include <pqxx/pqxx>
#include <string>

#include "log.h"

#define BUFF_SIZE 10

using std::string;

supla_voltage_aberration_logger_dao::supla_voltage_aberration_logger_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_electricity_logger_dao(dba) {}

void supla_voltage_aberration_logger_dao::add(const std::time_t &time,
                                              int channel_id, char phase,
                                              supla_simple_statiscics *stat,
                                              const std::string &procedure,
                                              unsigned char precision) {
  supla_abstract_electricity_logger_dao::add(time, channel_id, phase, stat,
                                             procedure, precision);
}

void supla_voltage_aberration_logger_dao::mariadb_add(
    const time_t &time, int channel_id, char phase, int count_total,
    int count_above, int count_below, int sec_above, int sec_below,
    int max_sec_above, int max_sec_below, char *min_voltage, char *max_voltage,
    char *avg_voltage, int measurement_time_sec) {
  MYSQL_BIND pbind[14] = {};
  MYSQL_TIME mytime = get_mdba()->time_t_to_mytime(&time);

  pbind[0].buffer_type = MYSQL_TYPE_DATETIME;
  pbind[0].buffer = (char *)&mytime;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&channel_id;

  pbind[2].buffer_type = MYSQL_TYPE_TINY;
  pbind[2].buffer = (char *)&phase;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&count_total;

  pbind[4].buffer_type = MYSQL_TYPE_LONG;
  pbind[4].buffer = (char *)&count_above;

  pbind[5].buffer_type = MYSQL_TYPE_LONG;
  pbind[5].buffer = (char *)&count_below;

  pbind[6].buffer_type = MYSQL_TYPE_LONG;
  pbind[6].buffer = (char *)&sec_above;

  pbind[7].buffer_type = MYSQL_TYPE_LONG;
  pbind[7].buffer = (char *)&sec_below;

  pbind[8].buffer_type = MYSQL_TYPE_LONG;
  pbind[8].buffer = (char *)&max_sec_above;

  pbind[9].buffer_type = MYSQL_TYPE_LONG;
  pbind[9].buffer = (char *)&max_sec_below;

  pbind[10].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[10].buffer = min_voltage;
  pbind[10].buffer_length = strnlen(min_voltage, BUFF_SIZE);

  pbind[11].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[11].buffer = max_voltage;
  pbind[11].buffer_length = strnlen(max_voltage, BUFF_SIZE);

  pbind[12].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[12].buffer = avg_voltage;
  pbind[12].buffer_length = strnlen(avg_voltage, BUFF_SIZE);

  pbind[13].buffer_type = MYSQL_TYPE_LONG;
  pbind[13].buffer = (char *)&measurement_time_sec;

  const char sql[] =
      "CALL "
      "`supla_add_em_voltage_aberration_log_item`(?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  get_mdba()->stmt_execute((void **)&stmt, sql, pbind, 14, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_voltage_aberration_logger_dao::tsdb_add(
    const time_t &time, int channel_id, short phase, int count_total,
    int count_above, int count_below, int sec_above, int sec_below,
    int max_sec_above, int max_sec_below, string min_voltage,
    string max_voltage, string avg_voltage, int measurement_time_sec) {
  pqxx::nontransaction ntx(*get_tsdba()->get_conn());

  ntx.exec_params(
      "SELECT "
      "supla_add_em_voltage_aberration_log_item($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,"
      "$11,$12,$13,$14)",
      get_tsdba()->time_to_timestamp_string(time), channel_id, phase,
      count_total, count_above, count_below, sec_above, sec_below,
      max_sec_above, max_sec_below, min_voltage, max_voltage, avg_voltage,
      measurement_time_sec);
}

void supla_voltage_aberration_logger_dao::add(
    const time_t &time, int channel_id, char phase,
    supla_voltage_aberration_analyzer *va) {
  if (!va || (va->get_below_count() == 0 && va->get_above_count() == 0)) {
    return;
  }

  int count_total = va->get_sample_count();
  int count_above = va->get_above_count();
  int count_below = va->get_below_count();

  int sec_above = va->get_total_msec_above() / 1000;
  int sec_below = va->get_total_msec_below() / 1000;
  int max_sec_above = va->get_max_msec_above() / 1000;
  int max_sec_below = va->get_max_msec_below() / 1000;

  char min_voltage[BUFF_SIZE] = {};
  snprintf(min_voltage, sizeof(min_voltage), "%0.2f", va->get_min());

  char max_voltage[BUFF_SIZE] = {};
  snprintf(max_voltage, sizeof(max_voltage), "%0.2f", va->get_max());

  char avg_voltage[BUFF_SIZE] = {};
  snprintf(avg_voltage, sizeof(avg_voltage), "%0.2f", va->get_avg());

  int measurement_time_sec = va->get_total_time_msec() / 1000;

  if (get_mdba()) {
    mariadb_add(time, channel_id, phase, count_total, count_above, count_below,
                sec_above, sec_below, max_sec_above, max_sec_below, min_voltage,
                max_voltage, avg_voltage, measurement_time_sec);
  } else if (get_tsdba()) {
    try {
      tsdb_add(time, channel_id, phase, count_total, count_above, count_below,
               sec_above, sec_below, max_sec_above, max_sec_below, min_voltage,
               max_voltage, avg_voltage, measurement_time_sec);
    } catch (const std::exception &e) {
      get_tsdba()->log_exception(e);
    }
  }
}

void supla_voltage_aberration_logger_dao::add(supla_electricity_analyzer *vas) {
  if (!vas) {
    return;
  }

  time_t now = std::time(nullptr);

  add(now, vas->get_channel_id(), 1, vas->get_aberration_phase1());
  add(now, vas->get_channel_id(), 2, vas->get_aberration_phase2());
  add(now, vas->get_channel_id(), 3, vas->get_aberration_phase3());
}
