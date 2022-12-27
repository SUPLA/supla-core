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

#include "integration/datalogger/ElectricityLoggerDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

ElectricityLoggerDaoIntegrationTest::ElectricityLoggerDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

ElectricityLoggerDaoIntegrationTest::~ElectricityLoggerDaoIntegrationTest() {}

void ElectricityLoggerDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_electricity_logger_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void ElectricityLoggerDaoIntegrationTest::TearDown() {
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

TEST_F(ElectricityLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_em_log", &result);

  EXPECT_EQ(result, "count\n0\n");

  TElectricityMeter_ExtendedValue_V2 em_ev = {};

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
      "WHERE date >= DATE_ADD(UTC_TIMESTAMP(), INTERVAL -2 SECOND) AND date <= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2 SECOND)",
      &result);

  EXPECT_EQ(result,
            "channel_id\tphase1_fae\tphase1_rae\tphase1_fre\tphase1_"
            "rre\tphase2_fae\tphase2_rae\tphase2_fre\tphase2_rre\tphase3_"
            "fae\tphase3_rae\tphase3_fre\tphase3_rre\tfae_balanced\trae_"
            "balanced\n10\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\n");
}

} /* namespace testing */
