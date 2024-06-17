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

#include "CurrentLoggerDaoIntegrationTest.h"

#include <string>

#include "analyzer/electricity_analyzer.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

using std::string;

namespace testing {

CurrentLoggerDaoIntegrationTest::CurrentLoggerDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

CurrentLoggerDaoIntegrationTest::~CurrentLoggerDaoIntegrationTest() {}

void CurrentLoggerDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_current_logger_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void CurrentLoggerDaoIntegrationTest::TearDown() {
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

TEST_F(CurrentLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_current_log", &result);

  EXPECT_EQ(result, "count\n0\n");

  electricity_meter_config config;
  config.set_user_config("{\"currentLoggerEnabled\":true}");

  supla_electricity_analyzer vas(234);

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
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
      "WHERE date >= DATE_ADD(UTC_TIMESTAMP(), INTERVAL -2 SECOND) AND date <= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2 SECOND)",
      &result);

  EXPECT_EQ(
      result,
      "channel_id\tphase_no\tmin\tmax\tavg\n234\t1\t10.510\t41.055\t25."
      "782\n234\t2\t15.520\t41.555\t28.538\n234\t3\t20.530\t42.055\t31.293\n");
}

} /* namespace testing */
