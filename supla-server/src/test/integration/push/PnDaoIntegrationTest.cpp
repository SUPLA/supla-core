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

#include "integration/push/PnDaoIntegrationTest.h"

#include <string>

using std::string;

namespace testing {

PnDaoIntegrationTest::PnDaoIntegrationTest() : IntegrationTest(), Test() {
  dba = nullptr;
  dao = nullptr;
}

PnDaoIntegrationTest::~PnDaoIntegrationTest() {}

void PnDaoIntegrationTest::SetUp() {
  Test::SetUp();

  dba = new supla_db_access_provider();
  ASSERT_TRUE(dba != nullptr);
  dao = new supla_pn_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void PnDaoIntegrationTest::TearDown() {
  Test::TearDown();

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

TEST_F(PnDaoIntegrationTest, get) {
  runSqlScript("AddPushNotification.sql");
  supla_push_notification pn(5);
  EXPECT_TRUE(dao->get(2, &pn));
  EXPECT_EQ(pn.get_id(), 5);
  EXPECT_EQ(pn.get_title(), "Abcd");
  EXPECT_EQ(pn.get_body(), "Efgh");
  EXPECT_EQ(pn.get_sound(), 11);
}

} /* namespace testing */
