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

#include "integration/datalogger/GeneralPurposeMeasurementLoggerDaoIntegrationTest.h"

#include <string>

#include "device/value/channel_general_purpose_measurement_value.h"

using std::string;

namespace testing {

GeneralPurposeMeasurementLoggerDaoIntegrationTest::
    GeneralPurposeMeasurementLoggerDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

GeneralPurposeMeasurementLoggerDaoIntegrationTest::
    ~GeneralPurposeMeasurementLoggerDaoIntegrationTest() {}

void GeneralPurposeMeasurementLoggerDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_general_purpose_measurement_logger_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void GeneralPurposeMeasurementLoggerDaoIntegrationTest::TearDown() {
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

TEST_F(GeneralPurposeMeasurementLoggerDaoIntegrationTest, add) {
  ASSERT_TRUE(dba->connect());

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_gp_measurement_log", &result);

  EXPECT_EQ(result, "count\n0\n");

  supla_general_purpose_measurement_analyzer analyzer(234);
  supla_channel_general_purpose_measurement_value value;

  value.set_value(123.34);
  analyzer.add_sample(&value);

  value.set_value(4);
  analyzer.add_sample(&value);

  value.set_value(15);
  analyzer.add_sample(&value);

  dao->add(&analyzer);

  result = "";

  sqlQuery(
      "SELECT channel_id, open_value, close_value, avg_value, max_value, "
      "min_value FROM supla_gp_measurement_log WHERE date >= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL -2 SECOND) AND date <= "
      "DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2 SECOND)",
      &result);

  EXPECT_EQ(result,
            "channel_id\topen_value\tclose_value\tavg_value\tmax_value\tmin_"
            "value\n234\t123.34\t15\t47.446666666666665\t123.34\t4\n");
}

} /* namespace testing */
