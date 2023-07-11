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

#include "PnLimitExceededTest.h"

#include "push/pn_limit_exceeded.h"

namespace testing {

using std::map;
using std::string;
using std::vector;

PnLimitExceededTest::PnLimitExceededTest(void) {}

PnLimitExceededTest::~PnLimitExceededTest(void) {}

TEST_F(PnLimitExceededTest, initializationWithTheSourceMessage) {
  supla_push_notification *src = new supla_push_notification(20);
  src->set_title("title");
  src->set_body("body {a}");
  src->set_localized_title("ltitle");
  src->set_localized_body("lbody");

  vector<string> args{"n", "m"};
  map<string, string> replacement_map{{"a", "b"}};

  src->set_localized_title_args(args);
  src->set_localized_body_args(args);
  src->set_sound(15);
  src->get_recipients().add(new supla_pn_recipient(10, 0, false, "ancd"),
                            platform_android);
  src->set_replacement_map(&replacement_map);

  supla_pn_limit_exceeded push(src, 150);
  args = push.get_localized_body_args();

  EXPECT_EQ(push.get_id(), 20);
  EXPECT_EQ(push.get_recipients().total_count(), 1);
  EXPECT_EQ(push.get_body(), "");
  EXPECT_EQ(push.get_title(), "");
  EXPECT_EQ(push.get_sound(), 0);
  EXPECT_EQ(push.get_localized_title(), "");
  EXPECT_EQ(push.get_localized_title_args().size(), 0);
  EXPECT_EQ(push.get_localized_body(), "push_notification_limit_exceeded");
  ASSERT_EQ(args.size(), 1);
  EXPECT_EQ(args.at(0), "150");

  push.get_recipients().add(new supla_pn_recipient(20, 0, false, "xyz"),
                            platform_android);
  EXPECT_EQ(push.get_recipients().total_count(), 2);
}

TEST_F(PnLimitExceededTest, immutable) {
  supla_pn_limit_exceeded push(new supla_push_notification(20), 30);

  push.set_title("title");
  push.set_body("body {a}");
  push.set_localized_title("ltitle");
  push.set_localized_body("lbody");

  vector<string> args{"n", "m"};
  map<string, string> replacement_map{{"a", "b"}};

  push.set_localized_title_args(args);
  push.set_localized_body_args(args);
  push.set_sound(15);
  push.get_recipients().add(new supla_pn_recipient(10, 0, false, "ancd"),
                            platform_android);
  push.set_replacement_map(replacement_map);

  args = push.get_localized_body_args();

  EXPECT_EQ(push.get_id(), 20);
  EXPECT_EQ(push.get_recipients().total_count(), 1);
  EXPECT_EQ(push.get_body(), "");
  EXPECT_EQ(push.get_title(), "");
  EXPECT_EQ(push.get_sound(), 0);
  EXPECT_EQ(push.get_localized_title(), "");
  EXPECT_EQ(push.get_localized_title_args().size(), 0);
  EXPECT_EQ(push.get_localized_body(), "push_notification_limit_exceeded");
  ASSERT_EQ(args.size(), 1);
  EXPECT_EQ(args.at(0), "30");
}

} /* namespace testing */
