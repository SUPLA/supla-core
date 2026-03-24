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

#include "hp_thermostat_logger_dao.h"

#include <mysql.h>
#include <stdio.h>
#include <string.h>

#define BUFF_SIZE 20

supla_hp_thermostat_logger_dao::supla_hp_thermostat_logger_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_cyclictask_dao(dba) {}

void supla_hp_thermostat_logger_dao::mariadb_add(int channel_id,
                                                 char *measured_temperature,
                                                 char *preset_temperature,
                                                 char on) {
  MYSQL_BIND pbind[4] = {};
  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = (char *)measured_temperature;
  pbind[1].buffer_length = strnlen(measured_temperature, BUFF_SIZE);

  pbind[2].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[2].buffer = (char *)preset_temperature;
  pbind[2].buffer_length = strnlen(preset_temperature, BUFF_SIZE);

  pbind[3].buffer_type = MYSQL_TYPE_TINY;
  pbind[3].buffer = (char *)&on;
  pbind[3].buffer_length = sizeof(char);

  const char sql[] = "CALL `supla_add_thermostat_log_item`(?,?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  get_mdba()->stmt_execute((void **)&stmt, sql, pbind, 4, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_hp_thermostat_logger_dao::tsdb_add(int channel_id,
                                              char *measured_temperature,
                                              char *preset_temperature,
                                              bool on) {
  pqxx::nontransaction ntx(*get_tsdba()->get_conn());

  ntx.exec_params("SELECT supla_add_thermostat_log_item($1,$2,$3,$4)",
                  channel_id, measured_temperature, preset_temperature, on);
}

void supla_hp_thermostat_logger_dao::add(
    int channel_id, supla_channel_hp_thermostat_value *th) {
  if (!th || !channel_id) {
    return;
  }

  char measured_temperature[BUFF_SIZE] = {};
  char preset_temperature[BUFF_SIZE] = {};

  snprintf(measured_temperature, BUFF_SIZE, "%05.2f",
           th->get_measured_temperature());
  snprintf(preset_temperature, BUFF_SIZE, "%05.2f",
           th->get_preset_temperature());

  if (get_mdba()) {
    mariadb_add(channel_id, measured_temperature, preset_temperature,
                th->is_on() ? 1 : 0);
  } else if (get_tsdba()) {
    try {
      tsdb_add(channel_id, measured_temperature, preset_temperature,
               th->is_on());
    } catch (const std::exception &e) {
      get_tsdba()->log_exception(e, channel_id);
    }
  }
}
