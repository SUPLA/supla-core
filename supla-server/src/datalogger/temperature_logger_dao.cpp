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

#include "datalogger/temperature_logger_dao.h"

#include <mysql.h>
#include <stdio.h>
#include <string.h>

#include <pqxx/pqxx>
#include <vector>

#include "device/value/channel_temphum_value.h"
#include "log.h"
#include "tools.h"

using std::vector;

#define BUFF_SIZE 20

supla_temperature_logger_dao::supla_temperature_logger_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_cyclictask_dao(dba) {}

void supla_temperature_logger_dao::mariadb_add_temperature(int channel_id,
                                                           char *temperature) {
  MYSQL_BIND pbind[2] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = temperature;
  pbind[1].buffer_length = strnlen(temperature, BUFF_SIZE);

  const char sql[] = "CALL `supla_add_temperature_log_item`(?,?)";

  MYSQL_STMT *stmt = nullptr;
  get_mdba()->stmt_execute((void **)&stmt, sql, pbind, 2, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_temperature_logger_dao::tsdb_add_temperature(int channel_id,
                                                        char *temperature) {
  pqxx::nontransaction ntx(*get_tsdba()->get_conn());

  ntx.exec_params("SELECT supla_add_temperature_log_item($1,$2)", channel_id,
                  temperature);
}

void supla_temperature_logger_dao::add_temperature(int channel_id,
                                                   double temperature) {
  if (channel_id == 0) {
    return;
  }

  char temperature_str[BUFF_SIZE] = {};
  if (0 != format_decimal_trunc(temperature, 8, 4, temperature_str,
                                sizeof(temperature_str))) {
    return;
  }

  if (get_mdba()) {
    mariadb_add_temperature(channel_id, temperature_str);
  } else if (get_tsdba()) {
    try {
      tsdb_add_temperature(channel_id, temperature_str);
    } catch (const std::exception &e) {
      get_tsdba()->log_exception(e, channel_id);
    }
  }
}

void supla_temperature_logger_dao::mariadb_add_temperature_and_humidity(
    int channel_id, char *temperature, char *humidity) {
  MYSQL_BIND pbind[3] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = temperature;
  pbind[1].buffer_length = strnlen(temperature, BUFF_SIZE);

  pbind[2].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[2].buffer = humidity;
  pbind[2].buffer_length = strnlen(humidity, BUFF_SIZE);

  const char sql[] = "CALL `supla_add_temphumidity_log_item`(?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  get_mdba()->stmt_execute((void **)&stmt, sql, pbind, 3, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_temperature_logger_dao::tsdb_add_temperature_and_humidity(
    int channel_id, char *temperature, char *humidity) {
  pqxx::nontransaction ntx(*get_tsdba()->get_conn());

  ntx.exec_params("SELECT supla_add_temphumidity_log_item($1,$2,$3)",
                  channel_id, temperature, humidity);
}

void supla_temperature_logger_dao::add_temperature_and_humidity(
    int channel_id, double temperature, double humidity) {
  if (channel_id == 0) {
    return;
  }

  char temperature_str[BUFF_SIZE] = {};
  char humidity_str[BUFF_SIZE] = {};

  if (0 != format_decimal_trunc(temperature, 8, 4, temperature_str,
                                sizeof(temperature_str))) {
    return;
  }

  if (0 != format_decimal_trunc(humidity, 8, 4, humidity_str,
                                sizeof(humidity_str))) {
    return;
  }

  if (get_mdba()) {
    mariadb_add_temperature_and_humidity(channel_id, temperature_str,
                                         humidity_str);
  } else if (get_tsdba()) {
    try {
      tsdb_add_temperature_and_humidity(channel_id, temperature_str,
                                        humidity_str);
    } catch (const std::exception &e) {
      get_tsdba()->log_exception(e, channel_id);
    }
  }
}

void supla_temperature_logger_dao::load(
    int user_id, std::vector<supla_abstract_channel_value_envelope *> *result) {
  if (result == nullptr || user_id == 0) {
    return;
  }

  vector<supla_abstract_channel_value_envelope *> unique;

  MYSQL_STMT *stmt = nullptr;
  const char sql[] =
      "SELECT c.id, c.type, c.func, v.value FROM `supla_dev_channel` c, "
      "`supla_dev_channel_value` v WHERE c.is_virtual = 0 AND c.user_id = ? "
      "AND c.id = v.channel_id AND v.valid_to >= UTC_TIMESTAMP() AND (c.func = "
      "? OR c.func = ? OR c.func = ?) GROUP BY c.id";

  int func1 = SUPLA_CHANNELFNC_THERMOMETER;
  int func2 = SUPLA_CHANNELFNC_HUMIDITY;
  int func3 = SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;

  MYSQL_BIND pbind[4] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_LONG;
  pbind[1].buffer = (char *)&func1;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&func2;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&func3;

  if (get_mdba()->stmt_execute((void **)&stmt, sql, pbind, 4, true)) {
    MYSQL_BIND rbind[4] = {};
    char value[SUPLA_CHANNELVALUE_SIZE] = {};
    int channel_id = 0;
    int type = 0;

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&channel_id;
    rbind[0].buffer_length = sizeof(int);

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&type;
    rbind[1].buffer_length = sizeof(int);

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&func1;
    rbind[2].buffer_length = sizeof(int);

    rbind[3].buffer_type = MYSQL_TYPE_BLOB;
    rbind[3].buffer = value;
    rbind[3].buffer_length = SUPLA_CHANNELVALUE_SIZE;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          supla_abstract_channel_value_envelope *env = nullptr;

          for (auto it = result->cbegin(); it != result->cend(); ++it) {
            env = *it;

            if (env != nullptr && env->get_channel_id() == channel_id) {
              break;
            } else {
              env = nullptr;
            }
          }

          if (env == nullptr) {
            env = new supla_abstract_channel_value_envelope(
                channel_id,
                new supla_channel_temphum_value(type, func1, value));
            unique.push_back(env);
          }

          memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  if (unique.size()) {
    result->insert(result->end(), unique.begin(), unique.end());
  }
}
