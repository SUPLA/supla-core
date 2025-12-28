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
