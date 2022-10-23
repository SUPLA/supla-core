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

#include "integration/datalogger/TemperatureLoggerDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

TemperatureLoggerDaoIntegrationTest::TemperatureLoggerDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

TemperatureLoggerDaoIntegrationTest::~TemperatureLoggerDaoIntegrationTest() {}

void TemperatureLoggerDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_temperature_logger_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void TemperatureLoggerDaoIntegrationTest::TearDown() {
  if (dao) {
    delete dao;
    dao = nullptr;
  }

  if (dba) {
    delete dba;
    dba = nullptr;
  }

  Test::TearDown();
}

TEST_F(TemperatureLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_temperature_log", &result);

  EXPECT_EQ(result, "count\n0\n");

  dao->add_temperature(15, 22.35);

  result = "";

  sqlQuery(
      "SELECT channel_id, temperature FROM supla_temperature_log WHERE date >= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL -2 SECOND) AND date <= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2 SECOND)",
      &result);

  EXPECT_EQ(result, "channel_id\ttemperature\n15\t22.3500\n");
}

} /* namespace testing */
