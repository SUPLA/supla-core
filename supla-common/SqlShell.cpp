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

#include "SqlShell.h"

#include <gtest/gtest.h>  // NOLINT

#include <cstdio>
#include <string>

#include "log.h"
#include "tools.h"

namespace testing {

using std::string;

// static
void SqlShell::runSqlScript(const char *sql_dir, const char *program,
                            const char *db_host, const char *user_switch,
                            const char *db_user, const char *db_name,
                            const char *script) {
  ASSERT_FALSE(script == NULL);
  ASSERT_GT(strlen(script), (unsigned long)0);
  char path[500];

  if (sql_dir == NULL) {
    snprintf(path, sizeof(path), "./%s", script);
  } else {
    snprintf(path, sizeof(path), "%s/%s", sql_dir, script);
  }

  if (st_file_exists(path) == 0) {
    supla_log(LOG_ERR, "File %s not exists!", path);
    ASSERT_TRUE(false);
  }

  char command[1000];
  snprintf(command, sizeof(command), "%s %s %s -h %s %s < %s", program,
           user_switch, db_user, db_host, db_name, path);
  supla_log(LOG_DEBUG, "%s", command);

  ASSERT_EQ(system(command), 0);
}

// static
void SqlShell::sqlQuery(const char *sql_dir, const char *program,
                        const char *db_host, const char *user_switch,
                        const char *db_user, const char *db_name,
                        const char *query, string *result) {
  ASSERT_FALSE(query == NULL);
  ASSERT_GT(strlen(query), (unsigned long)0);

  char command[1000];
  snprintf(command, sizeof(command), "echo \"%s\" | %s %s %s -h %s %s", query,
           program, user_switch, db_user, db_host, db_name);
  supla_log(LOG_DEBUG, "%s", command);

  char buff[1024];
  buff[0] = 0;

  FILE *command_fp = popen(command, "r");
  ASSERT_TRUE(command_fp != NULL);

  while (fgets(buff, sizeof(buff), command_fp) != NULL) {
    result->append(buff);
  }

  ASSERT_EQ(pclose(command_fp), 0);
}

}  // namespace testing
