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

#ifndef DBCOMMON_H_
#define DBCOMMON_H_

#define DB_VERSION "20240924120951"

#include <atomic>

class dbcommon {
 private:
#ifdef __DEBUG
  static std::atomic<int> conn_count;
#endif /*__DEBUG*/

 protected:
  void *_mysql;
  int query(const char *stmt_str, bool log_err = false);
  bool stmt_execute(void **_stmt, const char *stmt_str, void *bind,
                    int bind_size, bool exec_errors = false);
  bool stmt_get_int(void **_stmt, int *value1, int *value2, int *value3,
                    int *value4, const char *stmt_str, void *bind,
                    int bind_size, bool exec_errors = false);
  void stmt_close(void *_stmt);
  bool get_string(int id, char *buffer, unsigned int buffer_size, bool *is_null,
                  const char *sql);
  int get_int(int ID, int default_value, const char *sql);
  int get_count(int ID, const char *sql);

  int add_by_proc_call(const char *stmt_str, void *bind, int bind_size);

  virtual char *cfg_get_host(void) = 0;
  virtual char *cfg_get_user(void) = 0;
  virtual char *cfg_get_password(void) = 0;
  virtual char *cfg_get_database(void) = 0;
  virtual int cfg_get_port(void) = 0;

 public:
  dbcommon();
  bool connect(int connection_timeout_sec);
  bool connect(void);
  bool is_connected(void);
  void disconnect(void);
  virtual ~dbcommon();

  static bool mainthread_init(void);
  static void mainthread_end(void);

  void start_transaction(void);
  void commit(void);
  void rollback(void);

  int get_last_insert_id(void);
  bool get_db_version(char *buffer, int buffer_size);
  bool check_db_version(const char *expected_version,
                        int connection_timeout_sec);
  void set_terminating_byte(char *result_str, int buffer_size, int data_size,
                            bool is_null);
};

#endif /* DBCOMMON_H_ */
