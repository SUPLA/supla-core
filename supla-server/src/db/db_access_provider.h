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

#ifndef DBACCESS_PROVIDER_H_
#define DBACCESS_PROVIDER_H_

#include "db/abstract_db_access_provider.h"
#include "db/svrdb.h"

class supla_db_access_provider : public supla_abstract_db_access_provider,
                                 private svrdb {
 public:
  supla_db_access_provider(void);
  virtual ~supla_db_access_provider(void);

  virtual bool connect(void);
  virtual bool is_connected(void);
  virtual void disconnect(void);

  virtual void start_transaction(void);
  virtual void commit(void);
  virtual void rollback(void);

  virtual void set_terminating_byte(char *result_str, int buffer_size,
                                    int data_size, bool is_null);

  virtual int query(const char *stmt_str, bool log_err = false);

  virtual bool stmt_execute(void **_stmt, const char *stmt_str, void *bind,
                            int bind_size, bool exec_errors = false);

  virtual bool stmt_get_int(void **_stmt, int *value1, int *value2, int *value3,
                            int *value4, const char *stmt_str, void *bind,
                            int bind_size, bool exec_errors = false);

  virtual void stmt_close(void *_stmt);

  virtual bool get_string(int id, char *buffer, unsigned int buffer_size,
                          bool *is_null, const char *sql);

  virtual int get_int(int ID, int default_value, const char *sql);

  virtual int get_count(int ID, const char *sql);

  virtual int add_by_proc_call(const char *stmt_str, void *bind, int bind_size);
};

#endif /* DBACCESS_PROVIDER_H_ */
