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

#include "mariadb_access_provider.h"

supla_mariadb_access_provider::supla_mariadb_access_provider(void)
    : supla_abstract_db_access_provider(), svrdb() {}

supla_mariadb_access_provider::~supla_mariadb_access_provider(void) {}

bool supla_mariadb_access_provider::connect(void) { return svrdb::connect(); }

bool supla_mariadb_access_provider::is_connected(void) {
  return svrdb::is_connected();
}

void supla_mariadb_access_provider::disconnect(void) { svrdb::disconnect(); }

void supla_mariadb_access_provider::start_transaction(void) {
  svrdb::start_transaction();
}

void supla_mariadb_access_provider::commit(void) { svrdb::commit(); }

void supla_mariadb_access_provider::rollback(void) { svrdb::rollback(); }

void supla_mariadb_access_provider::set_terminating_byte(char *result_str,
                                                         int buffer_size,
                                                         int data_size,
                                                         bool is_null) {
  svrdb::set_terminating_byte(result_str, buffer_size, data_size, is_null);
}

int supla_mariadb_access_provider::query(const char *stmt_str, bool log_err) {
  return svrdb::query(stmt_str, log_err);
}

bool supla_mariadb_access_provider::stmt_execute(void **_stmt,
                                                 const char *stmt_str,
                                                 void *bind, int bind_size,
                                                 bool exec_errors) {
  return svrdb::stmt_execute(_stmt, stmt_str, bind, bind_size, exec_errors);
}

bool supla_mariadb_access_provider::stmt_get_int(
    void **_stmt, int *value1, int *value2, int *value3, int *value4,
    const char *stmt_str, void *bind, int bind_size, bool exec_errors) {
  return svrdb::stmt_get_int(_stmt, value1, value2, value3, value4, stmt_str,
                             bind, bind_size, exec_errors);
}

void supla_mariadb_access_provider::stmt_close(void *_stmt) {
  svrdb::stmt_close(_stmt);
}

bool supla_mariadb_access_provider::get_string(int id, char *buffer,
                                               unsigned int buffer_size,
                                               bool *is_null, const char *sql) {
  return svrdb::get_string(id, buffer, buffer_size, is_null, sql);
}

int supla_mariadb_access_provider::get_int(int ID, int default_value,
                                           const char *sql) {
  return svrdb::get_int(ID, default_value, sql);
}

int supla_mariadb_access_provider::get_count(int ID, const char *sql) {
  return svrdb::get_count(ID, sql);
}

int supla_mariadb_access_provider::add_by_proc_call(const char *stmt_str,
                                                    void *bind, int bind_size) {
  return svrdb::add_by_proc_call(stmt_str, bind, bind_size);
}

time_t supla_mariadb_access_provider::mytime_to_time_t(const MYSQL_TIME *time) {
  if (!time) {
    return 0;
  }

  struct tm timeinfo = {};
  timeinfo.tm_year = time->year - 1900;
  timeinfo.tm_mon = time->month - 1;
  timeinfo.tm_mday = time->day;
  timeinfo.tm_hour = time->hour;
  timeinfo.tm_min = time->minute;
  timeinfo.tm_sec = time->second;
  timeinfo.tm_isdst = -1;

  return timegm(&timeinfo);
}

MYSQL_TIME supla_mariadb_access_provider::time_t_to_mytime(const time_t *time) {
  std::tm tm_utc{};
  gmtime_r(time, &tm_utc);

  MYSQL_TIME mt{};
  mt.year = tm_utc.tm_year + 1900;
  mt.month = tm_utc.tm_mon + 1;
  mt.day = tm_utc.tm_mday;
  mt.hour = tm_utc.tm_hour;
  mt.minute = tm_utc.tm_min;
  mt.second = tm_utc.tm_sec;

  mt.second_part = 0;  // microseconds
  mt.neg = false;
  mt.time_type = MYSQL_TIMESTAMP_DATETIME;

  return mt;
}
