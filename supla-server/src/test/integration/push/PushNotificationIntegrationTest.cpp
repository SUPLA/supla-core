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

#include "PushNotificationIntegrationTest.h"

#include <string>

#include "push/push_notification.h"

using std::string;

namespace testing {

PushNotificationIntegrationTest::PushNotificationIntegrationTest()
    : IntegrationTest(), Test() {}

PushNotificationIntegrationTest::~PushNotificationIntegrationTest() {}

void PushNotificationIntegrationTest::SetUp() {
  Test::SetUp();
  initTestDatabase();
  runSqlScript("AddPushNotification.sql");
}

TEST_F(PushNotificationIntegrationTest, applyJsonWithoutIdAndWithAidsOnly) {
  const char json[] =
      "{\"title\":\"Ttitle\",\"body\":\"bOdyyy\",\"sound\":33,\"recipients\":{"
      "\"aids\":[2,3]}}";

  supla_push_notification p;
  p.apply_json(2, json);

  EXPECT_EQ(p.get_id(), 0);
  EXPECT_EQ(p.get_title(), "Ttitle");
  EXPECT_EQ(p.get_body(), "bOdyyy");
  EXPECT_EQ(p.get_sound(), 33);
  ASSERT_EQ(p.get_recipients().total_count(), 5);
  EXPECT_EQ(p.get_recipients().get(0)->get_client_id(), 24);
  EXPECT_EQ(p.get_recipients().get(1)->get_client_id(), 31);
  EXPECT_EQ(p.get_recipients().get(2)->get_client_id(), 32);
  EXPECT_EQ(p.get_recipients().get(3)->get_client_id(), 33);
  EXPECT_EQ(p.get_recipients().get(4)->get_client_id(), 4);
}

TEST_F(PushNotificationIntegrationTest, applyJsonWithoutIdAndWithCidsOnly) {
  const char json[] =
      "{\"title\":\"Ttitle\",\"body\":\"bOdyyy\",\"sound\":33,\"recipients\":{"
      "\"cids\":[24,33]}}";

  supla_push_notification p;
  p.apply_json(2, json);

  EXPECT_EQ(p.get_id(), 0);
  EXPECT_EQ(p.get_title(), "Ttitle");
  EXPECT_EQ(p.get_body(), "bOdyyy");
  EXPECT_EQ(p.get_sound(), 33);
  ASSERT_EQ(p.get_recipients().total_count(), 2);
  EXPECT_EQ(p.get_recipients().get(0)->get_client_id(), 24);
  EXPECT_EQ(p.get_recipients().get(1)->get_client_id(), 33);
}

TEST_F(PushNotificationIntegrationTest, applyJsonWithoutId) {
  const char json[] =
      "{\"title\":\"Ttitle\",\"body\":\"bOdyyy\",\"sound\":35,\"recipients\":{"
      "\"cids\":[33],\"aids\":[2]}}";

  supla_push_notification p;
  p.apply_json(2, json);

  EXPECT_EQ(p.get_id(), 0);
  EXPECT_EQ(p.get_title(), "Ttitle");
  EXPECT_EQ(p.get_body(), "bOdyyy");
  EXPECT_EQ(p.get_sound(), 35);
  ASSERT_EQ(p.get_recipients().total_count(), 4);
  EXPECT_EQ(p.get_recipients().get(0)->get_client_id(), 24);
  EXPECT_EQ(p.get_recipients().get(1)->get_client_id(), 31);
  EXPECT_EQ(p.get_recipients().get(2)->get_client_id(), 33);
  EXPECT_EQ(p.get_recipients().get(3)->get_client_id(), 4);
}

TEST_F(PushNotificationIntegrationTest, applyJsonWithId) {
  const char json[] =
      "{\"id\":5,\"title\":\"Ttitle\",\"body\":\"bOdyyy\",\"sound\":33,"
      "\"recipients\":{\"cids\":[33],\"aids\":[2]}}";

  supla_push_notification p;
  p.apply_json(2, json);

  EXPECT_EQ(p.get_id(), 5);
  EXPECT_EQ(p.get_title(), "");
  EXPECT_EQ(p.get_body(), "");
  EXPECT_EQ(p.get_sound(), 0);
  EXPECT_EQ(p.get_recipients().total_count(), 0);
}

} /* namespace testing */
