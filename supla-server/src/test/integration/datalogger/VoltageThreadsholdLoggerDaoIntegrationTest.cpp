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

#include "integration/datalogger/VoltageThreadsholdLoggerDaoIntegrationTest.h"

#include <string>

#include "device/extended_value/channel_em_extended_value.h"
#include "srpc/srpc.h"

using std::string;

namespace testing {

VoltageThreadsholdLoggerDaoIntegrationTest::
    VoltageThreadsholdLoggerDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

VoltageThreadsholdLoggerDaoIntegrationTest::
    ~VoltageThreadsholdLoggerDaoIntegrationTest() {}

void VoltageThreadsholdLoggerDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_voltage_threshold_logger_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void VoltageThreadsholdLoggerDaoIntegrationTest::TearDown() {
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

TEST_F(VoltageThreadsholdLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_voltage_log", &result);

  EXPECT_EQ(result, "count\n0\n");

  electricity_meter_config config;
  config.set_user_config(
      "{\"lowerVoltageThreshold\":200,\"upperVoltageThreshold\":300}");

  supla_voltage_analyzers vas;
  vas.set_channel_id(234);

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  em_ev.m_count = 1;
  em_ev.m[0].voltage[0] = 31055;
  em_ev.m[0].voltage[1] = 31555;
  em_ev.m[0].voltage[2] = 32055;
  em_ev.measured_values = EM_VAR_VOLTAGE;

  supla_channel_em_extended_value em(&em_ev, "", 0);

  vas.add_samples(0, &config, &em);
  usleep(2000000);
  vas.add_samples(0, &config, &em);

  em_ev.m[0].voltage[0] = 1051;
  em_ev.m[0].voltage[1] = 1552;
  em_ev.m[0].voltage[2] = 2053;

  em.set_raw_value(&em_ev);

  vas.add_samples(0, &config, &em);
  usleep(1000000);
  vas.add_samples(0, &config, &em);

  dao->add(&vas);

  result = "";

  sqlQuery(
      "SELECT channel_id, phase_no, count_total, count_above, count_below, "
      "sec_above, sec_below, max_sec_above, max_sec_below, min_voltage, "
      "max_voltage, avg_voltage, measurement_time_sec FROM "
      "supla_em_voltage_log WHERE date >= DATE_ADD(UTC_TIMESTAMP(), INTERVAL "
      "-2 SECOND) AND date <= DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2 SECOND)",
      &result);

  EXPECT_EQ(result,
            "channel_id\tphase_no\tcount_total\tcount_above\tcount_below\tsec_"
            "above\tsec_below\tmax_sec_above\tmax_sec_below\tmin_voltage\tmax_"
            "voltage\tavg_voltage\tmeasurement_time_"
            "sec\n234\t1\t4\t1\t1\t2\t1\t2\t1\t10.51\t310.55\t160."
            "53\t3\n234\t2\t4\t1\t1\t2\t1\t2\t1\t15.52\t315.55\t165."
            "53\t3\n234\t3\t4\t1\t1\t2\t1\t2\t1\t20.53\t320.55\t170.54\t3\n");
}

} /* namespace testing */
