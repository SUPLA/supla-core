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

#include "TSDB_IntegrationTest.h"

#include <string>

#include "PgSqlShell.h"
#include "log.h"  // NOLINT
#include "svrcfg.h"

namespace testing {

TSDB_IntegrationTest::TSDB_IntegrationTest() {}

TSDB_IntegrationTest::~TSDB_IntegrationTest() {}

void TSDB_IntegrationTest::runSqlScript(const char *script) {
  PgSqlShell::runSqlScript(cfg_sql_dir, scfg_string(CFG_TSDB_HOST),
                           scfg_string(CFG_TSDB_USER), scfg_string(CFG_TSDB_DB),
                           script);
}

void TSDB_IntegrationTest::sqlQuery(const char *query, std::string *result) {
  PgSqlShell::sqlQuery(cfg_sql_dir, scfg_string(CFG_TSDB_HOST),
                       scfg_string(CFG_TSDB_USER), scfg_string(CFG_TSDB_DB),
                       query, result);
}

void TSDB_IntegrationTest::initTestDatabase() {
  void initTestDatabase(const char *sql_dir, const char *db_host,
                        const char *db_user, const char *db_name);
  PgSqlShell::initTestDatabase(cfg_sql_dir, scfg_string(CFG_TSDB_HOST),
                               scfg_string(CFG_TSDB_USER),
                               scfg_string(CFG_TSDB_DB));
}

} /* namespace testing */
