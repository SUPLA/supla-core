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

#include "TSDB_TotalEnergyLoggerDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

TSDB_TotalEnergyLoggerDaoIntegrationTest::
TSDB_TotalEnergyLoggerDaoIntegrationTest()
: TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_TotalEnergyLoggerDaoIntegrationTest::
~TSDB_TotalEnergyLoggerDaoIntegrationTest() {}

void TSDB_TotalEnergyLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_total_energy_logger_dao(dba);
}

void TSDB_TotalEnergyLoggerDaoIntegrationTest::TearDown() {
  delete dao;
  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_TotalEnergyLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  TElectricityMeter_ExtendedValue_V3 em_ev = {};

  em_ev.total_forward_active_energy[0] = 1;
  em_ev.total_reverse_active_energy[0] = 2;
  em_ev.total_forward_reactive_energy[0] = 3;
  em_ev.total_reverse_reactive_energy[0] = 4;
  em_ev.total_forward_active_energy[1] = 5;
  em_ev.total_reverse_active_energy[1] = 6;
  em_ev.total_forward_reactive_energy[1] = 7;
  em_ev.total_reverse_reactive_energy[1] = 8;
  em_ev.total_forward_active_energy[2] = 9;
  em_ev.total_reverse_active_energy[2] = 10;
  em_ev.total_forward_reactive_energy[2] = 11;
  em_ev.total_reverse_reactive_energy[2] = 12;

  em_ev.total_forward_active_energy_balanced = 13;
  em_ev.total_reverse_active_energy_balanced = 14;

  dao->add(10, &em_ev);

  result = "";

  sqlQuery(
      "SELECT channel_id, phase1_fae, phase1_rae, phase1_fre, phase1_rre, "
      "phase2_fae, phase2_rae, phase2_fre, phase2_rre, phase3_fae, phase3_rae, "
      "phase3_fre, phase3_rre, fae_balanced, rae_balanced FROM supla_em_log "
      "WHERE date >= (now() AT TIME ZONE 'UTC') - INTERVAL '2 seconds' AND "
      "date <= (now() AT TIME ZONE 'UTC') + INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | phase1_fae | phase1_rae | phase1_fre | phase1_rre | phase2_fae | phase2_rae | phase2_fre | phase2_rre | phase3_fae | phase3_rae | phase3_fre | phase3_rre | fae_balanced | rae_balanced \n" // NOLINT
            "------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+--------------+--------------\n" // NOLINT
            "         10 |          1 |          2 |          3 |          4 |          5 |          6 |          7 |          8 |          9 |         10 |         11 |         12 |           13 |           14\n"  // NOLINT
            "(1 row)\n\n"); // NOLINT
}

TEST_F(TSDB_TotalEnergyLoggerDaoIntegrationTest, nullIfZero) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  TElectricityMeter_ExtendedValue_V3 em_ev = {};

  em_ev.total_reverse_active_energy[0] = 2;
  em_ev.total_reverse_reactive_energy[0] = 4;
  em_ev.total_reverse_active_energy[1] = 6;
  em_ev.total_reverse_reactive_energy[1] = 8;
  em_ev.total_reverse_active_energy[2] = 10;
  em_ev.total_reverse_reactive_energy[2] = 12;
  em_ev.total_reverse_active_energy_balanced = 14;

  dao->add(10, &em_ev);

  em_ev = {};

  em_ev.total_forward_active_energy[0] = 1;
  em_ev.total_forward_reactive_energy[0] = 3;
  em_ev.total_forward_active_energy[1] = 5;
  em_ev.total_forward_reactive_energy[1] = 7;
  em_ev.total_forward_active_energy[2] = 9;
  em_ev.total_forward_reactive_energy[2] = 11;
  em_ev.total_forward_active_energy_balanced = 13;

  dao->add(11, &em_ev);

  result = "";

  sqlQuery(
      "SELECT channel_id, phase1_fae, phase1_rae, phase1_fre, phase1_rre, "
      "phase2_fae, phase2_rae, phase2_fre, phase2_rre, phase3_fae, phase3_rae, "
      "phase3_fre, phase3_rre, fae_balanced, rae_balanced FROM supla_em_log "
      "WHERE date >= (now() AT TIME ZONE 'UTC') - INTERVAL '2 seconds' AND "
      "date <= (now() AT TIME ZONE 'UTC') + INTERVAL '2 seconds'",
      &result);

  EXPECT_EQ(result,
		  " channel_id | phase1_fae | phase1_rae | phase1_fre | phase1_rre | phase2_fae | phase2_rae | phase2_fre | phase2_rre | phase3_fae | phase3_rae | phase3_fre | phase3_rre | fae_balanced | rae_balanced \n" // NOLINT
		  "------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+------------+--------------+--------------\n" // NOLINT
		  "         10 |            |          2 |            |          4 |            |          6 |            |          8 |            |         10 |            |         12 |              |           14\n"  // NOLINT
		  "         11 |          1 |            |          3 |            |          5 |            |          7 |            |          9 |            |         11 |            |           13 |             \n"  // NOLINT
		  "(2 rows)\n\n"); // NOLINT
}

TEST_F(TSDB_TotalEnergyLoggerDaoIntegrationTest, AllZeros) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  TElectricityMeter_ExtendedValue_V3 em_ev = {};
  dao->add(10, &em_ev);

  result = "";

  sqlQuery("SELECT count(*) as count FROM supla_em_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");
}

TEST_F(TSDB_TotalEnergyLoggerDaoIntegrationTest, maxIntPlus1) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  TElectricityMeter_ExtendedValue_V3 em_ev = {};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
  em_ev.total_forward_active_energy[0] = INT64_MAX + 1;
#pragma GCC diagnostic pop

  dao->add(10, &em_ev);

  result = "";

  sqlQuery("SELECT count(*) as count FROM supla_em_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");
}

} /* namespace testing */
