// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MYSQLSHELL_H_
#define MYSQLSHELL_H_

#include <string>

#include "test/integration/SqlShell.h"

namespace testing {

class MySqlShell : public SqlShell {
 private:
  static const char *program;
  static const char *user_switch;

 public:
  static void runSqlScript(const char *sql_dir, const char *db_host,
                           const char *db_user, const char *db_name,
                           const char *script);
  static void sqlQuery(const char *sql_dir, const char *db_host,
                       const char *db_user, const char *db_name,
                       const char *query, std::string *result);

  static void initTestDatabase(const char *sql_dir, const char *db_host,
                               const char *db_user, const char *db_name);
};

}  // namespace testing

#endif /*MYSQLSHELL_*/
