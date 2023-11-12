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

#include "dbcommon.h"

#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "tools.h"

using std::atomic;

#ifdef __DEBUG
atomic<int> dbcommon::conn_count;
#endif /*__DEBUG*/

dbcommon::dbcommon() { _mysql = NULL; }

dbcommon::~dbcommon() { disconnect(); }

bool dbcommon::mainthread_init(void) {
  if (mysql_library_init(0, NULL, NULL)) {
    supla_log(LOG_ERR, "Could not initialize MySQL library");
    return false;
  }

  return true;
}

void dbcommon::mainthread_end(void) { mysql_library_end(); }

bool dbcommon::connect(int connection_timeout_sec) {
  if (is_connected()) {
    return true;
  }

  bool cmsg = false;
  struct timeval start_time;
  struct timeval tv;
  gettimeofday(&start_time, NULL);

  bool connected = false;

  do {
    if (_mysql == NULL) {
      _mysql = mysql_init(NULL);
    }

    if (_mysql == NULL) {
      supla_log(LOG_ERR, "MySQL initialization error");
      break;
    } else {
      if (mysql_real_connect((MYSQL *)_mysql, cfg_get_host(), cfg_get_user(),
                             cfg_get_password(), cfg_get_database(),
                             cfg_get_port(), NULL, 0) == NULL) {
#ifdef __DEBUG
        supla_log(LOG_ERR,
                  "Failed to connect to database. Conn count %i. Error: %s",
                  conn_count.load(), mysql_error((MYSQL *)_mysql));
#else
        supla_log(LOG_ERR, "Failed to connect to database: Error: %s",
                  mysql_error((MYSQL *)_mysql));
#endif /*__DEBUG*/
      } else {
        if (mysql_set_character_set((MYSQL *)_mysql, "utf8mb4")) {
          supla_log(LOG_ERR,
                    "Can't set utf8 character set. Current character set is %s",
                    mysql_character_set_name((MYSQL *)_mysql));
        }

        mysql_options((MYSQL *)_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");
        connected = true;
      }
    }

    if (!connected) {
      gettimeofday(&tv, NULL);
      if (connection_timeout_sec > 0) {
        if (!cmsg) {
          supla_log(LOG_INFO, "Connecting to database ....");
          cmsg = true;
        }
        usleep(1000000);
      }
    }
  } while (!connected && connection_timeout_sec > 0 && !st_app_terminate &&
           tv.tv_sec - start_time.tv_sec < connection_timeout_sec);

#ifdef __DEBUG
  if (connected) {
    conn_count++;
  }
#endif /*__DEBUG*/

  if (!connected) {
    supla_log(
        LOG_ERR,
        "MySQL - Failed to connect to database! Connection timeout %i sec.",
        connection_timeout_sec);

    if (_mysql != NULL) {
      mysql_close((MYSQL *)_mysql);
      _mysql = NULL;
    }

    return false;
  }

  return connected;
}

bool dbcommon::connect(void) { return connect(0); }

void dbcommon::disconnect(void) {
  if (_mysql != NULL) {
    mysql_close((MYSQL *)_mysql);
    _mysql = NULL;
#ifdef __DEBUG
    conn_count--;
#endif /*__DEBUG*/
  }
}

bool dbcommon::is_connected(void) { return _mysql != nullptr; }

int dbcommon::query(const char *stmt_str, bool log_err) {
  int result = mysql_query((MYSQL *)_mysql, stmt_str);

  if (result != 0 && log_err)
    supla_log(LOG_ERR, "MySQL - Query error %i: %s",
              mysql_errno((MYSQL *)_mysql), mysql_error((MYSQL *)_mysql));

  return result;
}

void dbcommon::start_transaction(void) { query("START TRANSACTION"); }

void dbcommon::commit(void) { query("COMMIT"); }

void dbcommon::rollback(void) { query("ROLLBACK", false); }

void dbcommon::stmt_close(void *_stmt) {
  if (_stmt != NULL) mysql_stmt_close((MYSQL_STMT *)_stmt);
}

bool dbcommon::stmt_execute(void **_stmt, const char *stmt_str, void *bind,
                            int bind_size, bool exec_errors) {
  *_stmt = NULL;
  MYSQL_STMT *stmt = mysql_stmt_init((MYSQL *)_mysql);

  if (stmt == NULL) {
    supla_log(LOG_ERR, "MySQL - mysql_stmt_init(), out of memory");

  } else if (mysql_stmt_prepare(stmt, stmt_str, strnlen(stmt_str, 10240))) {
    supla_log(LOG_ERR, "MySQL - stmt prepare error - %s",
              mysql_stmt_error(stmt));

  } else {
    bool err = false;

    if (bind != NULL) {
      int param_count = mysql_stmt_param_count(stmt);

      if (param_count != bind_size) {
        supla_log(LOG_ERR, "MySQL - invalid parameter count %i/%i", param_count,
                  bind_size);
        err = true;

      } else if (mysql_stmt_bind_param(stmt, (MYSQL_BIND *)bind) != 0) {
        supla_log(LOG_ERR, "MySQL - bind param error: %s",
                  mysql_stmt_error(stmt));
        err = true;
      }
    }

    if (err == false) {
      if (mysql_stmt_execute(stmt) != 0) {
        if (exec_errors)
          supla_log(LOG_ERR, "MySQL - execute error: %s. %s",
                    mysql_stmt_error(stmt), stmt_str);

      } else {
        *_stmt = stmt;
        return true;
      }
    }
  }

  if (exec_errors)
    mysql_stmt_close(stmt);
  else
    *_stmt = stmt;

  return false;
}

bool dbcommon::stmt_get_int(void **_stmt, int *value1, int *value2, int *value3,
                            int *value4, const char *stmt_str, void *bind,
                            int bind_size, bool exec_errors) {
  bool result = false;

  if (stmt_execute(_stmt, stmt_str, bind, bind_size, exec_errors)) {
    MYSQL_STMT *stmt = (MYSQL_STMT *)*_stmt;

    mysql_stmt_store_result(stmt);

    if (mysql_stmt_num_rows(stmt) > 0) {
      MYSQL_BIND bind_result[4];
      memset(bind_result, 0, sizeof(bind_result));

      bind_result[0].buffer_type = MYSQL_TYPE_LONG;
      bind_result[0].buffer = (char *)value1;

      if (value2 == NULL && value3 != NULL) {
        value2 = value3;
        value3 = NULL;
      }

      if (value2 == NULL && value4 != NULL) {
        value2 = value4;
        value4 = NULL;
      }

      if (value3 == NULL && value4 != NULL) {
        value3 = value4;
        value4 = NULL;
      }

      if (value2 != NULL) {
        bind_result[1].buffer_type = MYSQL_TYPE_LONG;
        bind_result[1].buffer = (char *)value2;
      }

      if (value3 != NULL) {
        bind_result[2].buffer_type = MYSQL_TYPE_LONG;
        bind_result[2].buffer = (char *)value3;
      }

      if (value4 != NULL) {
        bind_result[3].buffer_type = MYSQL_TYPE_LONG;
        bind_result[3].buffer = (char *)value4;
      }

      mysql_stmt_bind_result(stmt, bind_result);

      if (mysql_stmt_fetch(stmt) == 0) result = true;
    }

    mysql_stmt_free_result(stmt);
  }

  if (*_stmt) mysql_stmt_close((MYSQL_STMT *)*_stmt);

  return result;
}

int dbcommon::get_int(int ID, int default_value, const char *sql) {
  MYSQL_STMT *stmt = NULL;
  int Result = default_value;

  MYSQL_BIND pbind[1];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&ID;

  if (!stmt_get_int((void **)&stmt, &Result, NULL, NULL, NULL, sql, pbind, 1))
    return default_value;

  return Result;
}

bool dbcommon::get_string(int id, char *buffer, unsigned int buffer_size,
                          bool *is_null, const char *sql) {
  MYSQL_STMT *stmt = NULL;

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&id;

  bool result = false;

  if (stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    my_bool _is_null = false;
    unsigned long size = 0;

    MYSQL_BIND rbind[1];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = buffer;
    rbind[0].buffer_length = buffer_size;
    rbind[0].length = &size;
    rbind[0].is_null = &_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0 && !mysql_stmt_fetch(stmt) &&
          buffer_size > size) {
        buffer[size] = 0;
        *is_null = _is_null > 0;

        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}

int dbcommon::get_count(int ID, const char *sql) { return get_int(ID, 0, sql); }

int dbcommon::add_by_proc_call(const char *stmt_str, void *bind,
                               int bind_size) {
  int result = 0;
  MYSQL_STMT *stmt = NULL;

  char q_executed = 0;
  const char sql_var[] = "SET @id = 0";
  const char sql_sel[] = "SELECT @id";

  if (stmt_execute((void **)&stmt, sql_var, NULL, 0, true)) {
    if (stmt != NULL) mysql_stmt_close(stmt);
    q_executed++;
  }

  if (stmt_execute((void **)&stmt, stmt_str, bind, bind_size, true)) {
    if (stmt != NULL) mysql_stmt_close(stmt);
    q_executed++;
  }

  if (q_executed == 2 && stmt_execute((void **)&stmt, sql_sel, NULL, 0, true)) {
    MYSQL_BIND rbind[1];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&result;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);
      mysql_stmt_fetch(stmt);
    }
  }

  if (stmt != NULL) mysql_stmt_close(stmt);

  return result;
}

int dbcommon::get_last_insert_id(void) {
  MYSQL_STMT *stmt = NULL;
  int Result = 0;

  if (!stmt_get_int((void **)&stmt, &Result, NULL, NULL, NULL,
                    "SELECT LAST_INSERT_ID()", NULL, 0))
    return 0;

  return Result;
}

bool dbcommon::get_db_version(char *buffer, int buffer_size) {
  if (buffer == NULL || buffer_size < 15) {
    return false;
  }

  bool result = false;
  memset(buffer, 0, buffer_size);

  MYSQL_STMT *stmt = NULL;
  if (stmt_execute((void **)&stmt,
                   "SELECT MAX(REGEXP_REPLACE(version, '[a-zA-Z\\\\\\\\]+', "
                   "'')) FROM `migration_versions`",
                   NULL, 0, true)) {
    unsigned long size;
    MYSQL_BIND rbind[1];
    memset(rbind, 0, sizeof(rbind));

    rbind[0].buffer_type = MYSQL_TYPE_STRING;
    rbind[0].buffer = buffer;
    rbind[0].buffer_length = buffer_size;
    rbind[0].length = &size;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) == 1 && !mysql_stmt_fetch(stmt)) {
        result = true;
      }
    }

    mysql_stmt_close(stmt);
  }

  return result;
}

bool dbcommon::check_db_version(const char *expected_version,
                                int connection_timeout_sec) {
  if (expected_version == NULL || expected_version[0] == 0) {
    return true;
  }

  if (!connect(connection_timeout_sec)) {
    return false;
  }

  char version[15];
  if (!get_db_version(version, 15)) {
    disconnect();
    supla_log(LOG_ERR, "The version of the database can not be determined!");
  } else {
    disconnect();
    if (strncmp(version, expected_version, 14) == 0) {
      return true;
    } else {
      supla_log(LOG_ERR, "Incorrect database version! Expected: %s",
                expected_version);
    }
  }

  return false;
}

void dbcommon::set_terminating_byte(char *result_str, int buffer_size,
                                    int data_size, bool is_null) {
  if (is_null) {
    data_size = 0;
  }

  if (data_size >= buffer_size) {
    data_size = buffer_size - 1;
  }

  result_str[data_size] = 0;
}
