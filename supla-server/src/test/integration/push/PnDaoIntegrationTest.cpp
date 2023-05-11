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

TEST_F(PnDaoIntegrationTest, fieldsWithNullsShouldNotWriteTheObject) {
  std::srand(std::time(nullptr));

  string title = std::to_string(std::rand()) + "-Title";
  string body = std::to_string(std::rand()) + "-Body";
  int sound = std::rand();

  runSqlScript("AddPushNotificationWithNulls.sql");
  supla_push_notification pn(5);
  pn.set_title(title);
  pn.set_body(body);
  pn.set_sound(sound);

  EXPECT_TRUE(dao->get(2, &pn));
  EXPECT_EQ(pn.get_title(), title);
  EXPECT_EQ(pn.get_body(), body);
  EXPECT_EQ(pn.get_sound(), sound);

  runSqlScript("PushNotificationFillNulls.sql");

  EXPECT_TRUE(dao->get(2, &pn));
  EXPECT_EQ(pn.get_title(), "titleee");
  EXPECT_EQ(pn.get_body(), "bbody");
  EXPECT_EQ(pn.get_sound(), 155);
}

TEST_F(PnDaoIntegrationTest, getPushIdNotManagedByDeviceAndChannel) {
  runSqlScript("AddPushNotification.sql");

  int id = dao->get_device_managed_push_id(2, 73, 140);
  EXPECT_EQ(id, 0);
}

TEST_F(PnDaoIntegrationTest, getPushIdManagedByDevice) {
  runSqlScript("AddDeviceManagedPushNotifications.sql");

  int id = dao->get_device_managed_push_id(2, 83, 0);
  EXPECT_EQ(id, 16);
}

TEST_F(PnDaoIntegrationTest, getPushIdManagedByDeviceAndChannel) {
  runSqlScript("AddDeviceManagedPushNotifications.sql");

  int id = dao->get_device_managed_push_id(2, 83, 157);
  EXPECT_EQ(id, 15);
}

} /* namespace testing */
