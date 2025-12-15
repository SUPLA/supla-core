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

#include "TSDB_CurrentLoggerDaoIntegrationTest.h"

#include <string>

#include "analyzer/electricity_analyzer.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

using std::string;

namespace testing {

TSDB_CurrentLoggerDaoIntegrationTest::TSDB_CurrentLoggerDaoIntegrationTest()
    : TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_CurrentLoggerDaoIntegrationTest::~TSDB_CurrentLoggerDaoIntegrationTest() {}

void TSDB_CurrentLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_current_logger_dao(dba);
}

void TSDB_CurrentLoggerDaoIntegrationTest::TearDown() {
  delete dao;

  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_CurrentLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_current_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  electricity_meter_config config;
  config.set_user_config("{\"currentLoggerEnabled\":true}");

  supla_electricity_analyzer vas(234);

  TElectricityMeter_ExtendedValue_V3 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].current[0] = 41055;
  em_ev.m[0].current[1] = 41555;
  em_ev.m[0].current[2] = 42055;
  em_ev.measured_values = EM_VAR_CURRENT;

  supla_channel_em_extended_value em(&em_ev, "", 0);

  vas.add_sample(0, &config, &em);

  em_ev.m[0].current[0] = 10510;
  em_ev.m[0].current[1] = 15520;
  em_ev.m[0].current[2] = 20530;

  em.set_raw_value(&em_ev);

  vas.add_sample(0, &config, &em);

  dao->add(&vas);

  result = "";

  sqlQuery(
      "SELECT channel_id, phase_no, min, max, avg FROM supla_em_current_log "
      "WHERE date >= (now() AT TIME ZONE 'UTC') - INTERVAL '2 seconds' AND "
      "date <= (now() AT TIME ZONE 'UTC') + INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | phase_no |  min   |  max   |  avg   \n"
            "------------+----------+--------+--------+--------\n"
            "        234 |        1 | 10.510 | 41.055 | 25.782\n"
            "        234 |        2 | 15.520 | 41.555 | 28.538\n"
            "        234 |        3 | 20.530 | 42.055 | 31.293\n"
            "(3 rows)\n\n");
}

} /* namespace testing */
