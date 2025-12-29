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

#include "TSDB_PowerActiveLoggerDaoIntegrationTest.h"

#include <string>

#include "analyzer/electricity_analyzer.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

using std::string;

namespace testing {

TSDB_PowerActiveLoggerDaoIntegrationTest::
TSDB_PowerActiveLoggerDaoIntegrationTest()
: TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_PowerActiveLoggerDaoIntegrationTest::
~TSDB_PowerActiveLoggerDaoIntegrationTest() {}

void TSDB_PowerActiveLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_power_active_logger_dao(dba);
}

void TSDB_PowerActiveLoggerDaoIntegrationTest::TearDown() {
  delete dao;
  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_PowerActiveLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_power_active_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  electricity_meter_config config;
  config.set_user_config("{\"powerActiveLoggerEnabled\":true}");

  supla_electricity_analyzer vas(234);

  TElectricityMeter_ExtendedValue_V3 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].power_active[0] = 410551;
  em_ev.m[0].power_active[1] = 415552;
  em_ev.m[0].power_active[2] = 420553;
  em_ev.measured_values = EM_VAR_POWER_ACTIVE;

  supla_channel_em_extended_value em(&em_ev, "", 0);

  vas.add_sample(0, &config, &em);

  em_ev.m[0].power_active[0] = 105101;
  em_ev.m[0].power_active[1] = 155202;
  em_ev.m[0].power_active[2] = 205303;

  em.set_raw_value(&em_ev);

  vas.add_sample(0, &config, &em);

  dao->add(&vas);

  result = "";

  sqlQuery(
      "SELECT channel_id, phase_no, min, max, avg FROM "
      "supla_em_power_active_log WHERE date >= (now() AT TIME ZONE 'UTC') - "
      "INTERVAL '2 seconds' AND date <= (now() AT TIME ZONE 'UTC') + INTERVAL "
      "'2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | phase_no |   min   |   max   |   avg   \n"
            "------------+----------+---------+---------+---------\n"
            "        234 |        1 | 1.05101 | 4.10551 | 2.57826\n"
            "        234 |        2 | 1.55202 | 4.15552 | 2.85377\n"
            "        234 |        3 | 2.05303 | 4.20553 | 3.12928\n"
            "(3 rows)\n\n");
}

} /* namespace testing */
