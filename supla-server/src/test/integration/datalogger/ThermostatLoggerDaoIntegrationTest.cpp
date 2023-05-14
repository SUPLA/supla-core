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

#include "integration/datalogger/ThermostatLoggerDaoIntegrationTest.h"

#include <string>

#include "device/value/channel_thermostat_value.h"

using std::string;

namespace testing {

ThermostatLoggerDaoIntegrationTest::ThermostatLoggerDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

ThermostatLoggerDaoIntegrationTest::~ThermostatLoggerDaoIntegrationTest() {}

void ThermostatLoggerDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_thermostat_logger_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void ThermostatLoggerDaoIntegrationTest::TearDown() {
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

TEST_F(ThermostatLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_thermostat_log", &result);

  EXPECT_EQ(result, "count\n0\n");

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->IsOn = 1;
  ((TThermostat_Value *)raw_value)->PresetTemperature = 2234;
  ((TThermostat_Value *)raw_value)->MeasuredTemperature = 2134;

  supla_channel_thermostat_value tv(raw_value);

  dao->add(25, &tv);

  result = "";

  sqlQuery(
      "SELECT channel_id, \\`on\\`, measured_temperature, preset_temperature "
      "FROM "
      "supla_thermostat_log WHERE date >= DATE_ADD(UTC_TIMESTAMP(), INTERVAL "
      "-2 SECOND) AND date <= DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2 SECOND)",
      &result);

  EXPECT_EQ(result,
            "channel_id\ton\tmeasured_temperature\tpreset_"
            "temperature\n25\t1\t21.34\t22.34\n");
}

} /* namespace testing */
