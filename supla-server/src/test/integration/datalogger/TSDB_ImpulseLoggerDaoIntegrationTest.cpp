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

#include "TSDB_ImpulseLoggerDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

TSDB_ImpulseLoggerDaoIntegrationTest::TSDB_ImpulseLoggerDaoIntegrationTest()
    : TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_ImpulseLoggerDaoIntegrationTest::~TSDB_ImpulseLoggerDaoIntegrationTest() {}

void TSDB_ImpulseLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_impulse_logger_dao(dba);
}

void TSDB_ImpulseLoggerDaoIntegrationTest::TearDown() {
  delete dao;

  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_ImpulseLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_ic_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  TDS_ImpulseCounter_Value ic_val = {};

  ic_val.counter = 4;

  supla_json_config cfg;
  cfg.set_user_config("{\"impulsesPerUnit\":2}");

  supla_channel_ic_extended_value icv(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                      &ic_val, &cfg);

  dao->add(11, &icv);

  result = "";

  sqlQuery(
      "SELECT channel_id, counter, calculated_value FROM supla_ic_log WHERE  "
      "date >= (now() AT TIME ZONE 'UTC') - INTERVAL '2 seconds' AND date <= "
      "(now() AT TIME ZONE 'UTC') + INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | counter | calculated_value \n"
            "------------+---------+------------------\n"
            "         11 |       4 |             2000\n"
            "(1 row)\n\n");
}

} /* namespace testing */
