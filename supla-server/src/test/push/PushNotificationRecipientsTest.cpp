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

#include "PushNotificationRecipientsTest.h"

namespace testing {

PushNotificationRecipientsTest::PushNotificationRecipientsTest(void) {}

PushNotificationRecipientsTest::~PushNotificationRecipientsTest(void) {}

TEST_F(PushNotificationRecipientsTest, counting) {
  EXPECT_EQ(recipients.total_count(), 0);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 0);
  EXPECT_EQ(recipients.count(platform_ios), 0);

  recipients.add(new supla_pn_recipient(1, 0, false, "id1"), platform_android);

  EXPECT_EQ(recipients.total_count(), 1);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 1);
  EXPECT_EQ(recipients.count(platform_ios), 0);

  recipients.add(new supla_pn_recipient(2, 1, false, "id2"), platform_android);

  EXPECT_EQ(recipients.total_count(), 2);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 2);
  EXPECT_EQ(recipients.count(platform_ios), 0);

  recipients.add(new supla_pn_recipient(3, 2, false, "id1"), platform_ios);

  EXPECT_EQ(recipients.total_count(), 3);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 2);
  EXPECT_EQ(recipients.count(platform_ios), 1);

  recipients.add(new supla_pn_recipient(3, 0, false, "id1"), platform_ios);

  EXPECT_EQ(recipients.total_count(), 4);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 2);
  EXPECT_EQ(recipients.count(platform_ios), 2);
}

TEST_F(PushNotificationRecipientsTest, addAndGet) {
  recipients.add(new supla_pn_recipient(1, 0, false, "and1"), platform_android);
  recipients.add(new supla_pn_recipient(2, 0, true, "ios1"), platform_ios);
  recipients.add(new supla_pn_recipient(3, 5, false, "ios2"), platform_ios);

  supla_pn_recipient *r = recipients.get(platform_android, 1);
  EXPECT_TRUE(r == nullptr);

  r = recipients.get(platform_android, 0);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_token(), "and1");
  EXPECT_EQ(r->get_client_id(), 1);
  EXPECT_EQ(r->get_app_id(), 0);
  EXPECT_FALSE(r->is_development_env());

  r = recipients.get(platform_ios, 0);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_token(), "ios1");
  EXPECT_EQ(r->get_client_id(), 2);
  EXPECT_EQ(r->get_app_id(), 0);
  EXPECT_TRUE(r->is_development_env());

  r = recipients.get(platform_ios, 1);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_token(), "ios2");
  EXPECT_EQ(r->get_client_id(), 3);
  EXPECT_EQ(r->get_app_id(), 5);
  EXPECT_FALSE(r->is_development_env());

  r = recipients.get(platform_ios, 2);
  EXPECT_TRUE(r == nullptr);

  // The order of rising from the platform on the map

  r = recipients.get(0);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_token(), "ios1");
  EXPECT_EQ(r->get_client_id(), 2);
  EXPECT_EQ(r->get_app_id(), 0);
  EXPECT_TRUE(r->is_development_env());

  r = recipients.get(1);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_token(), "ios2");
  EXPECT_EQ(r->get_client_id(), 3);
  EXPECT_EQ(r->get_app_id(), 5);
  EXPECT_FALSE(r->is_development_env());

  r = recipients.get(2);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_token(), "and1");
  EXPECT_EQ(r->get_client_id(), 1);
  EXPECT_EQ(r->get_app_id(), 0);
  EXPECT_FALSE(r->is_development_env());

  r = recipients.get(3);
  EXPECT_TRUE(r == nullptr);
}

TEST_F(PushNotificationRecipientsTest, clear) {
  recipients.add(new supla_pn_recipient(1, 0, false, "and1"), platform_android);
  recipients.add(new supla_pn_recipient(2, 0, true, "ios1"), platform_ios);
  recipients.clear();
  EXPECT_EQ(recipients.total_count(), 0);
}

} /* namespace testing */
