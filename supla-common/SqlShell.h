// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SQLSHELL_H_
#define SQLSHELL_H_

#include <string>

namespace testing {

class SqlShell {
 protected:
  static void runSqlScript(const char *sql_dir, const char *program,
                           const char *db_host, const char *user_switch,
                           const char *db_user, const char *db_name,
                           const char *script);
  static void sqlQuery(const char *sql_dir, const char *program,
                       const char *db_host, const char *user_switch,
                       const char *db_user, const char *db_name,
                       const char *query, std::string *result);

  static void initTestDatabase(const char *sql_dir, const char *program,
                               const char *db_host, const char *user_switch,
                               const char *db_user, const char *db_name,
                               const char *recreate_script,
                               const char *structure_script);

 public:
};

}  // namespace testing

#endif /*SQLSHELL_*/
