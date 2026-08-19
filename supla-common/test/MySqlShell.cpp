// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "MySqlShell.h"

#include <gtest/gtest.h>  // NOLINT

#include "log.h"
#include "tools.h"

namespace testing {

using std::string;

const char *MySqlShell::program = "mysql";
const char *MySqlShell::user_switch = "--ssl=0 -u";

// static
void MySqlShell::runSqlScript(const char *sql_dir, const char *db_host,
                              const char *db_user, const char *db_name,
                              const char *script) {
  SqlShell::runSqlScript(sql_dir, program, db_host, user_switch, db_user,
                         db_name, script);
}

// static
void MySqlShell::sqlQuery(const char *sql_dir, const char *db_host,
                          const char *db_user, const char *db_name,
                          const char *query, string *result) {
  SqlShell::sqlQuery(sql_dir, program, db_host, user_switch, db_user, db_name,
                     query, result);
}

// static
void MySqlShell::initTestDatabase(const char *sql_dir, const char *db_host,
                                  const char *db_user, const char *db_name) {
  SqlShell::runSqlScript(sql_dir, program, db_host, user_switch, db_user,
                         db_name, "RecreateTestDatabase.sql");
  SqlShell::runSqlScript(sql_dir, program, db_host, user_switch, db_user,
                         db_name, "TestDatabaseStructureAndData.sql");
}

}  // namespace testing
