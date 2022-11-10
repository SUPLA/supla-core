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

#include <integration/scene/SceneDaoIntegrationTest.h>

#include "log.h"  // NOLINT

namespace testing {

SceneDaoIntegrationTest::SceneDaoIntegrationTest() : IntegrationTest(), Test() {
  dao = nullptr;
  dba = nullptr;
}

SceneDaoIntegrationTest::~SceneDaoIntegrationTest() {}

void SceneDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  dao = new supla_scene_dao(dba);
  ASSERT_TRUE(dao != NULL);

  initTestDatabase();
  Test::SetUp();
}

void SceneDaoIntegrationTest::TearDown() {
  if (dao) {
    delete dao;
    dao = NULL;
  }

  if (dba) {
    delete dba;
    dba = NULL;
  }
  Test::TearDown();
}

TEST_F(SceneDaoIntegrationTest, getEstimatedExecutionTime) {
  runSqlScript("SetSceneEstimatedExecutionTime.sql");
  EXPECT_EQ(123456, dao->get_estimated_execution_time(3));
}

} /* namespace testing */
