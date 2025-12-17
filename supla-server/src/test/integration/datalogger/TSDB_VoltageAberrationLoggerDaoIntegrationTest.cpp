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

#include "TSDB_VoltageAberrationLoggerDaoIntegrationTest.h"

#include <string>

#include "analyzer/electricity_analyzer.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

using std::string;

namespace testing {

TSDB_VoltageAberrationLoggerDaoIntegrationTest::
TSDB_VoltageAberrationLoggerDaoIntegrationTest()
: TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_VoltageAberrationLoggerDaoIntegrationTest::
~TSDB_VoltageAberrationLoggerDaoIntegrationTest() {}

void TSDB_VoltageAberrationLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_voltage_aberration_logger_dao(dba);
}

void TSDB_VoltageAberrationLoggerDaoIntegrationTest::TearDown() {
  delete dao;

  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_VoltageAberrationLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_voltage_aberration_log",
           &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":200,\"upperVoltageThreshold\":300}");

  supla_electricity_analyzer vas(234);

  TElectricityMeter_ExtendedValue_V3 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  supla_channel_em_extended_value em(&em_ev, "", 0);

  vas.add_sample(0, &config, &em);
  usleep(2000000);
  vas.add_sample(0, &config, &em);

  em_ev.m[0].voltage[0] = 1051;
  em_ev.m[0].voltage[1] = 1552;
  em_ev.m[0].voltage[2] = 2053;

  em.set_raw_value(&em_ev);

  vas.add_sample(0, &config, &em);
  usleep(1000000);
  vas.add_sample(0, &config, &em);

  dao->add(&vas);

  result = "";

  sqlQuery(
      "SELECT channel_id, phase_no, count_total, count_above, count_below, "
      "sec_above, sec_below, max_sec_above, max_sec_below, min_voltage, "
      "max_voltage, avg_voltage, measurement_time_sec FROM "
      "supla_em_voltage_aberration_log WHERE date >= (now() AT TIME ZONE "
      "'UTC') - INTERVAL '2 seconds' AND date <= (now() AT TIME ZONE 'UTC') + "
      "INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
  " channel_id | phase_no | count_total | count_above | count_below | sec_above | sec_below | max_sec_above | max_sec_below | min_voltage | max_voltage | avg_voltage | measurement_time_sec \n" // NOLINT
  "------------+----------+-------------+-------------+-------------+-----------+-----------+---------------+---------------+-------------+-------------+-------------+----------------------\n" // NOLINT
  "        234 |        1 |           4 |           1 |           1 |         2 |         1 |             2 |             1 |       10.51 |      310.55 |      160.53 |                    3\n"  // NOLINT
  "        234 |        2 |           4 |           1 |           1 |         2 |         1 |             2 |             1 |       15.52 |      315.55 |      165.53 |                    3\n"  // NOLINT
  "        234 |        3 |           4 |           1 |           1 |         2 |         1 |             2 |             1 |       20.53 |      320.55 |      170.54 |                    3\n"  // NOLINT
  "(3 rows)\n\n");
}

} /* namespace testing */
