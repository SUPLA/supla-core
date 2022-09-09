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

#include "conn/connection_dao.h"

#include <mysql.h>
#include <string.h>

#include "log.h"

supla_connection_dao::supla_connection_dao(
    supla_abstract_db_access_provider *dba)
    : supla_abstract_connection_dao() {
  this->dba = dba;
}

supla_connection_dao::~supla_connection_dao() {}

int supla_connection_dao::get_user_id_by_email(
    const char email[SUPLA_EMAIL_MAXSIZE]) {
  if (email == nullptr || strnlen(email, SUPLA_EMAIL_MAXSIZE) == 0) {
    return 0;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return 0;
  }

  int result = 0;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_STRING;
  pbind.buffer = (char *)email;
  pbind.buffer_length = strnlen(email, SUPLA_EMAIL_MAXSIZE);

  MYSQL_STMT *stmt = NULL;

  if (!dba->stmt_get_int((void **)&stmt, &result, NULL, NULL, NULL,
                         "SELECT id FROM supla_user WHERE email = ?", &pbind,
                         1)) {
    result = 0;
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

bool supla_connection_dao::get_reg_enabled(int user_id, unsigned int *client,
                                           unsigned int *iodevice) {
  if (user_id == 0) {
    return false;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  bool result = false;

  MYSQL_STMT *stmt = nullptr;

  MYSQL_BIND pbind[1] = {};

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  if (dba->stmt_get_int(
          (void **)&stmt, (int *)client, (int *)iodevice, nullptr, nullptr,
          "SELECT CAST(IFNULL(UNIX_TIMESTAMP(CONVERT_TZ(IF(client_reg_enabled "
          ">= UTC_TIMESTAMP(), client_reg_enabled, NULL),'+00:00','SYSTEM')), "
          "0) AS UNSIGNED INT), "
          "CAST(IFNULL(UNIX_TIMESTAMP(CONVERT_TZ(IF(iodevice_reg_enabled >= "
          "UTC_TIMESTAMP(), iodevice_reg_enabled, NULL),'+00:00','SYSTEM')), "
          "0) AS UNSIGNED INT) FROM supla_user WHERE id = ?",
          pbind, 1)) {
    result = true;
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}

bool supla_connection_dao::get_user_localtime(int user_id,
                                              TSDC_UserLocalTimeResult *time) {
  if (user_id == 0) {
    return false;
  }

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return false;
  }

  bool result = false;
  char sql[] =
      "SELECT @date := IFNULL(CONVERT_TZ(UTC_TIMESTAMP, 'UTC', timezone), "
      "UTC_TIMESTAMP) t, CASE CONVERT_TZ(UTC_TIMESTAMP, 'UTC', timezone) WHEN "
      "NULL THEN 'UTC' ELSE timezone END tz, YEAR(@date) y, MONTH(@date) m, "
      "DAY(@date) d, DAYOFWEEK(@date) w, HOUR(@date) h, MINUTE(@date) n, "
      "SECOND(@date) s FROM `supla_user` WHERE id = ?";

  memset(time, 0, sizeof(TSDC_UserLocalTimeResult));

  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  if (dba->stmt_execute((void **)&stmt, sql, pbind, 1, true)) {
    MYSQL_BIND rbind[9];
    memset(rbind, 0, sizeof(rbind));

    char date[30];
    unsigned long date_size = 0;
    my_bool date_is_null = true;

    unsigned long timezone_size = 0;
    my_bool timezone_is_null = true;

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = date;
    rbind[0].buffer_length = sizeof(date);
    rbind[0].length = &date_size;
    rbind[0].is_null = &date_is_null;

    rbind[1].buffer_type = MYSQL_TYPE_STRING;
    rbind[1].buffer = time->timezone;
    rbind[1].buffer_length = SUPLA_TIMEZONE_MAXSIZE;
    rbind[1].length = &timezone_size;
    rbind[1].is_null = &timezone_is_null;

    rbind[2].buffer_type = MYSQL_TYPE_SHORT;
    rbind[2].buffer = (char *)&time->year;

    rbind[3].buffer_type = MYSQL_TYPE_TINY;
    rbind[3].buffer = (char *)&time->month;

    rbind[4].buffer_type = MYSQL_TYPE_TINY;
    rbind[4].buffer = (char *)&time->day;

    rbind[5].buffer_type = MYSQL_TYPE_TINY;
    rbind[5].buffer = (char *)&time->dayOfWeek;

    rbind[6].buffer_type = MYSQL_TYPE_TINY;
    rbind[6].buffer = (char *)&time->hour;

    rbind[7].buffer_type = MYSQL_TYPE_TINY;
    rbind[7].buffer = (char *)&time->min;

    rbind[8].buffer_type = MYSQL_TYPE_TINY;
    rbind[8].buffer = (char *)&time->sec;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt) &&
          !timezone_is_null) {
        time->timezone[timezone_size] = 0;
        time->timezoneSize = timezone_size + 1;
        result = true;
      }
    }
    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}
