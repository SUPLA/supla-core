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

#include "TSDB_LoggerDaoIntegrationTest.h"

namespace testing {

TSDB_LoggerDaoIntegrationTest::TSDB_LoggerDaoIntegrationTest()
    : TSDB_IntegrationTest(), Test() {
  dba = nullptr;
}

TSDB_LoggerDaoIntegrationTest::~TSDB_LoggerDaoIntegrationTest() {}

void TSDB_LoggerDaoIntegrationTest::SetUp() {
  dba = new supla_tsdb_access_provider(
      "gssencmode=disable sslmode=disable");  // When connecting to SSL,
                                              // Valgrind warns after the
                                              // program exits that 259 bytes
                                              // are still available. This
                                              // doesn't appear to be a leak.
                                              // Therefore, let's disable SSL
                                              // and gssencmode for testing.
  initTestDatabase();
  Test::SetUp();
}

void TSDB_LoggerDaoIntegrationTest::TearDown() {
  delete dba;
  Test::TearDown();
}

} /* namespace testing */
