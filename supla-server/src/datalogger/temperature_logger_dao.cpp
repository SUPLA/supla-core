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

#include "device/value/channel_temphum_value.h"
#include "log.h"

using std::vector;

supla_temperature_logger_dao::supla_temperature_logger_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

void supla_temperature_logger_dao::add_temperature(int channel_id,
                                                   double temperature) {
  if (channel_id == 0) {
    return;
  }

  char buff[20] = {};
  MYSQL_BIND pbind[2] = {};

  snprintf(buff, sizeof(buff), "%04.4f", temperature);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = (char *)buff;
  pbind[1].buffer_length = strnlen(buff, 20);

  const char sql[] = "CALL `supla_add_temperature_log_item`(?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, pbind, 2, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_temperature_logger_dao::add_temperature_and_humidity(
    int channel_id, double temperature, double humidity) {
  if (channel_id == 0) {
    return;
  }

  char buff1[20] = {};
  char buff2[20] = {};
  MYSQL_BIND pbind[3] = {};

  snprintf(buff1, sizeof(buff1), "%04.4f", temperature);
  snprintf(buff2, sizeof(buff2), "%04.4f", humidity);

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&channel_id;

  pbind[1].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[1].buffer = (char *)buff1;
  pbind[1].buffer_length = strnlen(buff1, 20);

  pbind[2].buffer_type = MYSQL_TYPE_DECIMAL;
  pbind[2].buffer = (char *)buff2;
  pbind[2].buffer_length = strnlen(buff2, 20);

  const char sql[] = "CALL `supla_add_temphumidity_log_item`(?,?,?)";

  MYSQL_STMT *stmt = nullptr;
  dba->stmt_execute((void **)&stmt, sql, pbind, 3, true);

  if (stmt != nullptr) mysql_stmt_close(stmt);
}

void supla_temperature_logger_dao::load(
    int user_id, std::vector<supla_channel_value_envelope *> *result) {
  if (result == nullptr || user_id == 0) {
    return;
  }

  vector<supla_channel_value_envelope *> unique;

  MYSQL_STMT *stmt = nullptr;
  const char sql[] =
      "SELECT c.id, c.type, c.func, v.value FROM `supla_dev_channel` c, "
      "`supla_dev_channel_value` v WHERE c.user_id = ? AND c.id = v.channel_id "
      "AND v.valid_to >= UTC_TIMESTAMP() AND (c.func = ? OR c.func = ? OR "
      "c.func = ?) GROUP BY c.id";

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

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 4, true)) {
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
          supla_channel_value_envelope *env = nullptr;

          for (auto it = result->cbegin(); it != result->cend(); ++it) {
            env = *it;

            if (env != nullptr && env->get_channel_id() == channel_id) {
              break;
            } else {
              env = nullptr;
            }
          }

          if (env == nullptr) {
            env = new supla_channel_value_envelope(
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
