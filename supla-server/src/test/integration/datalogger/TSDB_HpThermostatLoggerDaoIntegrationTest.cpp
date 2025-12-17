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

#include "TSDB_HpThermostatLoggerDaoIntegrationTest.h"

#include <string>

#include "device/value/channel_hp_thermostat_value.h"

using std::string;

namespace testing {

TSDB_HpThermostatLoggerDaoIntegrationTest::
TSDB_HpThermostatLoggerDaoIntegrationTest()
: TSDB_LoggerDaoIntegrationTest() {
  dao = nullptr;
}

TSDB_HpThermostatLoggerDaoIntegrationTest::
~TSDB_HpThermostatLoggerDaoIntegrationTest() {}

void TSDB_HpThermostatLoggerDaoIntegrationTest::SetUp() {
  TSDB_LoggerDaoIntegrationTest::SetUp();
  dao = new supla_hp_thermostat_logger_dao(dba);
}

void TSDB_HpThermostatLoggerDaoIntegrationTest::TearDown() {
  delete dao;

  TSDB_LoggerDaoIntegrationTest::TearDown();
}

TEST_F(TSDB_HpThermostatLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_thermostat_log", &result);

  EXPECT_EQ(result, " count \n-------\n     0\n(1 row)\n\n");

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->IsOn = 1;
  ((TThermostat_Value *)raw_value)->PresetTemperature = 2234;
  ((TThermostat_Value *)raw_value)->MeasuredTemperature = 2134;

  supla_channel_hp_thermostat_value tv(raw_value);

  dao->add(25, &tv);

  result = "";

  sqlQuery(
      "SELECT channel_id, \\\"on\\\", measured_temperature, preset_temperature "
      "FROM supla_thermostat_log WHERE  date >= (now() AT TIME ZONE 'UTC') - "
      "INTERVAL '2 seconds' AND date <= (now() AT TIME ZONE 'UTC') + INTERVAL "
      "'2 seconds'",
      &result);

  EXPECT_EQ(result,
            " channel_id | on | measured_temperature | preset_temperature \n"
            "------------+----+----------------------+--------------------\n"
            "         25 | t  |                21.34 |              22.34\n"
            "(1 row)\n\n");
}

} /* namespace testing */
