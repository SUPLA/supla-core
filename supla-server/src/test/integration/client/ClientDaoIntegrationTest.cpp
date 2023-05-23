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

#include "ClientDaoIntegrationTest.h"

namespace testing {

using std::string;

ClientDaoIntegrationTest::ClientDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dao = nullptr;
  dba = nullptr;
}

ClientDaoIntegrationTest::~ClientDaoIntegrationTest() {}

void ClientDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  dao = new supla_client_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void ClientDaoIntegrationTest::TearDown() {
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

TEST_F(ClientDaoIntegrationTest,
       setPushNotificationClientTokenAndRemoveWithEmptyString) {
  string result;
  sqlQuery(
      "SELECT id FROM supla_client WHERE id = 1 AND user_id = 2 AND push_token "
      "IS NULL AND platform IS NULL AND app_id = 0 AND devel_env = 0",
      &result);

  EXPECT_EQ(result, "id\n1\n");
  result = "";
  dao->update_client_push_notification_client_token(2, 1, "abcd", 2, 3, true);

  sqlQuery(
      "SELECT push_token,platform,app_id,devel_env FROM supla_client WHERE id "
      "= 1",
      &result);
  EXPECT_EQ(result, "push_token\tplatform\tapp_id\tdevel_env\nabcd\t2\t3\t1\n");
  result = "";

  dao->update_client_push_notification_client_token(2, 1, "", 1, 2, false);

  sqlQuery(
      "SELECT push_token,platform,app_id,devel_env FROM supla_client WHERE id "
      "= 1",
      &result);
  EXPECT_EQ(result, "push_token\tplatform\tapp_id\tdevel_env\nNULL\t1\t2\t0\n");
}

TEST_F(ClientDaoIntegrationTest, removePushNotificationClientTokenWithNull) {
  dao->update_client_push_notification_client_token(2, 1, "abcd", 2, 3, true);
  dao->update_client_push_notification_client_token(2, 1, nullptr, 2, 3, true);

  string result;
  sqlQuery(
      "SELECT push_token,platform,app_id,devel_env FROM supla_client WHERE id "
      "= 1",
      &result);
  EXPECT_EQ(result, "push_token\tplatform\tapp_id\tdevel_env\nNULL\t2\t3\t1\n");
}

} /* namespace testing */
