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

#include "TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest.h"

#include <string>

#include "device/value/channel_general_purpose_meter_value.h"

using std::string;

namespace testing {

TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest::
TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest()
: TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest::
~TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest() {}

void TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_general_purpose_meter_logger_dao(dba);
}

void TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest::TearDown() {
  delete dao;

  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_GeneralPurposeMeterLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_gp_meter_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  supla_channel_general_purpose_meter_value value;
  value.set_value(123.34);
  dao->add(234, &value);

  result = "";

  sqlQuery(
      "SELECT channel_id, value FROM supla_gp_meter_log WHERE date >= (now() "
      "AT TIME ZONE 'UTC') - INTERVAL '2 seconds' AND date <= (now() AT TIME "
      "ZONE 'UTC') + INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | value  \n"
            "------------+--------\n"
            "        234 | 123.34\n"
            "(1 row)\n\n");
}

} /* namespace testing */
