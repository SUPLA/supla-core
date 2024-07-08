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

#include "PowerActiveLoggerDaoIntegrationTest.h"

#include <string>

#include "analyzer/electricity_analyzer.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

using std::string;

namespace testing {

PowerActiveLoggerDaoIntegrationTest::PowerActiveLoggerDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

PowerActiveLoggerDaoIntegrationTest::~PowerActiveLoggerDaoIntegrationTest() {}

void PowerActiveLoggerDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_power_active_logger_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void PowerActiveLoggerDaoIntegrationTest::TearDown() {
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

TEST_F(PowerActiveLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_power_active_log", &result);

  EXPECT_EQ(result, "count\n0\n");

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
      "supla_em_power_active_log WHERE date >= DATE_ADD(UTC_TIMESTAMP(), "
      "INTERVAL -2 SECOND) AND date <= DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2 "
      "SECOND)",
      &result);

  EXPECT_EQ(result,
            "channel_id\tphase_no\tmin\tmax\tavg\n234\t1\t1.05101\t4.10551\t2."
            "57826\n234\t2\t1.55202\t4.15552\t2.85377\n234\t3\t2.05303\t4."
            "20553\t3.12928\n");
}

} /* namespace testing */
