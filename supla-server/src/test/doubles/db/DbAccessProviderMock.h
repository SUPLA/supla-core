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

#ifndef DB_ACCESS_PROVIDER_MOCK_H_
#define DB_ACCESS_PROVIDER_MOCK_H_

#include <gmock/gmock.h>

#include "db/abstract_db_access_provider.h"

namespace testing {

class DbAccessProviderMock : public supla_abstract_db_access_provider {
 public:
  DbAccessProviderMock(void);
  virtual ~DbAccessProviderMock(void);

  MOCK_METHOD0(connect, bool(void));

  MOCK_METHOD0(is_connected, bool(void));

  MOCK_METHOD0(disconnect, void(void));

  MOCK_METHOD0(start_transaction, void(void));

  MOCK_METHOD0(commit, void(void));

  MOCK_METHOD0(rollback, void(void));

  MOCK_METHOD4(set_terminating_byte, void(char *result_str, int buffer_size,
                                          int data_size, bool is_null));

  MOCK_METHOD2(query, int(const char *stmt_str, bool log_err));

  MOCK_METHOD5(stmt_execute, bool(void **_stmt, const char *stmt_str,
                                  void *bind, int bind_size, bool exec_errors));

  MOCK_METHOD9(stmt_get_int,
               bool(void **_stmt, int *value1, int *value2, int *value3,
                    int *value4, const char *stmt_str, void *bind,
                    int bind_size, bool exec_errors));

  MOCK_METHOD1(stmt_close, void(void *_stmt));

  MOCK_METHOD3(get_int, int(int ID, int default_value, const char *sql));

  MOCK_METHOD2(get_count, int(int ID, const char *sql));

  MOCK_METHOD3(add_by_proc_call,
               int(const char *stmt_str, void *bind, int bind_size));
};

} /* namespace testing */

#endif /* DB_ACCESS_PROVIDER_MOCK_H_ */
