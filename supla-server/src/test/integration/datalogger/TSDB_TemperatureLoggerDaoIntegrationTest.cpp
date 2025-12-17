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

#include "TSDB_TemperatureLoggerDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

TSDB_TemperatureLoggerDaoIntegrationTest::
TSDB_TemperatureLoggerDaoIntegrationTest()
: TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_TemperatureLoggerDaoIntegrationTest::
~TSDB_TemperatureLoggerDaoIntegrationTest() {}

void TSDB_TemperatureLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_temperature_logger_dao(dba);
}

void TSDB_TemperatureLoggerDaoIntegrationTest::TearDown() {
  delete dao;
  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_TemperatureLoggerDaoIntegrationTest, addTemperature) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_temperature_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  dao->add_temperature(15, 22.3567);

  result = "";

  sqlQuery(
      "SELECT channel_id, temperature FROM supla_temperature_log WHERE date >= "
      "(now() AT TIME ZONE 'UTC') - INTERVAL '2 seconds' AND date <= (now() AT "
      "TIME ZONE 'UTC') + INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | temperature \n"
            "------------+-------------\n"
            "         15 |     22.3567\n"
            "(1 row)\n\n");
}

TEST_F(TSDB_TemperatureLoggerDaoIntegrationTest, addTemperatureAndHumidity) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_temphumidity_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  dao->add_temperature_and_humidity(17, 36.2341, 80.5671);
  result = "";

  sqlQuery(
      "SELECT channel_id, temperature, humidity FROM supla_temphumidity_log "
      "WHERE date >= (now() AT TIME ZONE 'UTC') - INTERVAL '2 seconds' AND "
      "date <= (now() AT TIME ZONE 'UTC') + INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | temperature | humidity \n"
            "------------+-------------+----------\n"
            "         17 |     36.2341 |  80.5671\n"
            "(1 row)\n\n");
}

} /* namespace testing */
