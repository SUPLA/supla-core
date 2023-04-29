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

#include "integration/schedule/ScheduleDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

ScheduleDaoIntegrationTest::ScheduleDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

ScheduleDaoIntegrationTest::~ScheduleDaoIntegrationTest() {}

void ScheduleDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_schedule_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void ScheduleDaoIntegrationTest::TearDown() {
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

TEST_F(ScheduleDaoIntegrationTest, disableAndEnable) {
  string result;

  sqlQuery("SELECT enabled FROM supla_schedule WHERE id = 1 AND user_id = 2",
           &result);

  EXPECT_EQ(result, "enabled\n1\n");

  result = "";

  dao->enable(2, 2, false);

  sqlQuery("SELECT enabled FROM supla_schedule WHERE id = 1 AND user_id = 2",
           &result);

  EXPECT_EQ(result, "enabled\n1\n");

  result = "";

  dao->enable(1, 1, false);

  sqlQuery("SELECT enabled FROM supla_schedule WHERE id = 1 AND user_id = 2",
           &result);

  EXPECT_EQ(result, "enabled\n1\n");

  result = "";

  dao->enable(2, 1, false);

  sqlQuery("SELECT enabled FROM supla_schedule WHERE id = 1 AND user_id = 2",
           &result);

  EXPECT_EQ(result, "enabled\n0\n");

  result = "";

  dao->enable(2, 1, true);

  sqlQuery("SELECT enabled FROM supla_schedule WHERE id = 1 AND user_id = 2",
           &result);

  EXPECT_EQ(result, "enabled\n1\n");
}

} /* namespace testing */
