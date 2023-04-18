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

  recipients.add(new supla_push_notification_recipient(app_supla, "id1"),
                 platform_android);

  EXPECT_EQ(recipients.total_count(), 1);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 1);
  EXPECT_EQ(recipients.count(platform_ios), 0);

  recipients.add(new supla_push_notification_recipient(app_nice, "id2"),
                 platform_android);

  EXPECT_EQ(recipients.total_count(), 2);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 2);
  EXPECT_EQ(recipients.count(platform_ios), 0);

  recipients.add(new supla_push_notification_recipient(app_supla, "id1"),
                 platform_ios);

  EXPECT_EQ(recipients.total_count(), 3);
  EXPECT_EQ(recipients.count(platform_unknown), 0);
  EXPECT_EQ(recipients.count(platform_android), 2);
  EXPECT_EQ(recipients.count(platform_ios), 1);
}

TEST_F(PushNotificationRecipientsTest, addAndGet) {
  recipients.add(new supla_push_notification_recipient(app_supla, "and1"),
                 platform_android);
  recipients.add(new supla_push_notification_recipient(app_supla, "ios1"),
                 platform_ios);
  recipients.add(new supla_push_notification_recipient(app_supla, "ios2"),
                 platform_ios);

  supla_push_notification_recipient *r = recipients.get(platform_android, 1);
  EXPECT_TRUE(r == nullptr);

  r = recipients.get(platform_android, 0);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_id(), "and1");

  r = recipients.get(platform_ios, 0);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_id(), "ios1");

  r = recipients.get(platform_ios, 1);
  ASSERT_TRUE(r != nullptr);
  EXPECT_EQ(r->get_id(), "ios2");

  r = recipients.get(platform_ios, 2);
  EXPECT_TRUE(r == nullptr);
}

} /* namespace testing */
