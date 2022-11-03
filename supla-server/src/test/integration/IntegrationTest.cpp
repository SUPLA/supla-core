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

#include "IntegrationTest.h"

#include "MySqlShell.h"
#include "log.h"  // NOLINT
#include "svrcfg.h"

namespace testing {

IntegrationTest::IntegrationTest() {}

IntegrationTest::~IntegrationTest() {}

void IntegrationTest::runSqlScript(const char *script) {
  MySqlShell::runSqlScript(cfg_sql_dir, scfg_string(CFG_MYSQL_HOST),
                           scfg_string(CFG_MYSQL_USER),
                           scfg_string(CFG_MYSQL_DB), script);
}

void IntegrationTest::sqlQuery(const char *query, std::string *result) {
  MySqlShell::sqlQuery(cfg_sql_dir, scfg_string(CFG_MYSQL_HOST),
                       scfg_string(CFG_MYSQL_USER), scfg_string(CFG_MYSQL_DB),
                       query, result);
}

void IntegrationTest::initTestDatabase() {
  MySqlShell::initTestDatabase(cfg_sql_dir, scfg_string(CFG_MYSQL_HOST),
                               scfg_string(CFG_MYSQL_USER),
                               scfg_string(CFG_MYSQL_DB));
}

} /* namespace testing */
