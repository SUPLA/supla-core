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

#include "PushNotificationTest.h"

#include "push/push_notification.h"

namespace testing {

using std::map;
using std::string;

PushNotificationTest::PushNotificationTest(void) {}

PushNotificationTest::~PushNotificationTest(void) {}

TEST_F(PushNotificationTest, voidContructor) {
  supla_push_notification n;
  EXPECT_EQ(n.get_id(), 0);
  EXPECT_EQ(n.get_recipients().total_count(), 0);
  EXPECT_EQ(n.get_title(), "");
  EXPECT_EQ(n.get_body(), "");
  EXPECT_EQ(n.get_sound(), 0);
}

TEST_F(PushNotificationTest, constructorWithId) {
  supla_push_notification n(10);
  EXPECT_EQ(n.get_id(), 10);
  EXPECT_EQ(n.get_recipients().total_count(), 0);
  EXPECT_EQ(n.get_title(), "");
  EXPECT_EQ(n.get_body(), "");
  EXPECT_EQ(n.get_sound(), 0);
}

TEST_F(PushNotificationTest, titleSetterAndGetter) {
  supla_push_notification n;
  n.set_title("asd");
  EXPECT_EQ(n.get_title(), "asd");
}

TEST_F(PushNotificationTest, bodySetterAndGetter) {
  supla_push_notification n;
  n.set_title("uijh");
  EXPECT_EQ(n.get_title(), "uijh");
}

TEST_F(PushNotificationTest, soundSetterAndGetter) {
  supla_push_notification n;
  n.set_sound(123);
  EXPECT_EQ(n.get_sound(), 123);
}

TEST_F(PushNotificationTest, replacementMapLoopPrevention) {
  supla_push_notification n;
  map<string, string> m;
  m["A"] = "{A}";

  n.set_body("{A}{A}{A}");
  n.set_replacement_map(m);
  EXPECT_EQ(n.get_body(), "{A}{A}{A}");
}

TEST_F(PushNotificationTest, replacementMapWithBody) {
  supla_push_notification n;
  map<string, string> m;
  m["HUMIDITY"] = "45.50";
  m["TEMPERATURE"] = "25.5";

  n.set_body(
      "The air humidity is {HUMIDITY}% and the temperature is {TEMPERATURE} "
      "degrees Celsius.");
  n.set_replacement_map(m);

  EXPECT_EQ(n.get_body(),
            "The air humidity is 45.50% and the temperature is 25.5 degrees "
            "Celsius.");

  m["ABCD"] = "123456";
  n.set_replacement_map(m);

  n.set_body("{ABCD}{ABCD} {ABCD}");

  EXPECT_EQ(n.get_body(), "123456123456 123456");
}

TEST_F(PushNotificationTest, replacementMapWithTtile) {
  supla_push_notification n;
  map<string, string> m;
  m["XYZ"] = "1";

  n.set_title("{XYZ}{XYZ}");
  n.set_replacement_map(m);
  EXPECT_EQ(n.get_title(), "11");

  m["A"] = "B";
  n.set_replacement_map(m);
  n.set_title("{A}");
  EXPECT_EQ(n.get_title(), "B");
}

} /* namespace testing */
