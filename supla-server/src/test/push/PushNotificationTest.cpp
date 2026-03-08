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

#include <string>
#include <vector>

#include "push/push_notification.h"

namespace testing {

using std::map;
using std::string;
using std::vector;

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
  n.set_body("uijh");
  EXPECT_EQ(n.get_body(), "uijh");
}

TEST_F(PushNotificationTest, localizedTitleSetterAndGetter) {
  supla_push_notification n;
  n.set_localized_title("lasd");
  EXPECT_EQ(n.get_localized_title(), "lasd");
}

TEST_F(PushNotificationTest, localizedBodySetterAndGetter) {
  supla_push_notification n;
  n.set_localized_body("luijh");
  EXPECT_EQ(n.get_localized_body(), "luijh");
}

TEST_F(PushNotificationTest, localizedTitleArgsSetterAndGetter) {
  supla_push_notification n;
  vector<string> args_in{"a", "b"};
  n.set_localized_title_args(args_in);

  vector<string> args_out = n.get_localized_title_args();
  ASSERT_EQ(args_out.size(), 2);
  EXPECT_EQ(args_out.at(0), "a");
  EXPECT_EQ(args_out.at(1), "b");
}

TEST_F(PushNotificationTest, localizedBodyArgsSetterAndGetter) {
  supla_push_notification n;
  vector<string> args_in{"c", "d"};
  n.set_localized_body_args(args_in);

  vector<string> args_out = n.get_localized_body_args();
  ASSERT_EQ(args_out.size(), 2);
  EXPECT_EQ(args_out.at(0), "c");
  EXPECT_EQ(args_out.at(1), "d");
}

TEST_F(PushNotificationTest, soundSetterAndGetter) {
  supla_push_notification n;
  n.set_sound(123);
  EXPECT_EQ(n.get_sound(), 123);
}

TEST_F(PushNotificationTest, templateDataMapLoopPrevention) {
  supla_push_notification n;
  nlohmann::json d;
  d["A"] = "{A}";

  n.set_body("{A}{A}{A}");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_body(), "{A}{A}{A}");
}

TEST_F(PushNotificationTest, templateDataWithBody) {
  supla_push_notification n;
  nlohmann::json d;
  d["HUMIDITY"] = "45.50";
  d["TEMPERATURE"] = "25.5";

  n.set_body(
      "The air humidity is {HUMIDITY}% and the temperature is {TEMPERATURE} "
      "degrees Celsius.");
  n.set_template_data(&d);

  EXPECT_EQ(n.get_body(),
            "The air humidity is 45.50% and the temperature is 25.5 degrees "
            "Celsius.");

  d["ABCD"] = "123456";
  n.set_template_data(&d);

  n.set_body("{ABCD}{ABCD} {ABCD}");

  EXPECT_EQ(n.get_body(), "123456123456 123456");
}

TEST_F(PushNotificationTest, templateDatatMapWithTtile) {
  supla_push_notification n;
  nlohmann::json d;
  d["XYZ"] = "1";

  n.set_title("{XYZ}{XYZ}");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_title(), "11");

  d["A"] = "B";
  n.set_template_data(&d);
  n.set_title("{A}");
  EXPECT_EQ(n.get_title(), "B");
}

TEST_F(PushNotificationTest, templateDataWithLocalizedTtile) {
  supla_push_notification n;
  nlohmann::json d;
  d["XY"] = "1";

  n.set_localized_title("{XY}{XY}");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_localized_title(), "11");

  d["A"] = "B";
  n.set_template_data(&d);
  n.set_localized_title("{A}");
  EXPECT_EQ(n.get_localized_title(), "B");
}

TEST_F(PushNotificationTest, templateMapWithLocalizedBody) {
  supla_push_notification n;
  nlohmann::json d;
  d["XM"] = "1";

  n.set_localized_body("{XM}{XM}");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_localized_body(), "11");

  d["U"] = "B";
  n.set_template_data(&d);
  n.set_localized_body("{U}");
  EXPECT_EQ(n.get_localized_body(), "B");
}

TEST_F(PushNotificationTest, dateTime) {
  nlohmann::json d;
  d["XY"] = "11";

  {
    supla_push_notification n;
    n.set_title("{XY}|{date}|{time}|{date_time}");
    n.set_template_data(&d);
    EXPECT_EQ(n.get_title(), "11|{date}|{time}|{date_time}");
    n.set_date_time("2023-08-13 13:53:14");
    EXPECT_EQ(n.get_title(), "11|2023-08-13|13:53:14|2023-08-13 13:53:14");
  }

  {
    supla_push_notification n;
    n.set_title("{XY}|{date}|{time}|{date_time}");
    n.set_date_time("2023-08-13 13:53:14");

    EXPECT_EQ(n.get_title(), "{XY}|2023-08-13|13:53:14|2023-08-13 13:53:14");

    n.set_template_data(&d);
    EXPECT_EQ(n.get_title(), "11|2023-08-13|13:53:14|2023-08-13 13:53:14");
  }

  {
    supla_push_notification n;

    n.set_date_time("2023-08-13 13:53:14");
    n.set_template_data(&d);

    n.set_title("{XY}|{date}|{time}|{date_time}");
    EXPECT_EQ(n.get_title(), "11|2023-08-13|13:53:14|2023-08-13 13:53:14");
  }
}

TEST_F(PushNotificationTest, titleWithInja) {
  supla_push_notification n;
  nlohmann::json d;
  d["state"] = "1";

  string tmpl =
      "{{ state }}: {% if state == \"1\" %}ONE{% else %}NOT ONE{% endif %}";

  n.set_title(tmpl);
  n.set_template_data(&d);
  EXPECT_EQ(n.get_title(), "1: ONE");

  d["state"] = "2";
  n.set_template_data(&d);
  EXPECT_EQ(n.get_title(), "2: NOT ONE");
}

TEST_F(PushNotificationTest, bodyWithInja) {
  supla_push_notification n;
  nlohmann::json d;
  d["str"] = "abcd";

  n.set_body("Length: {{ length(str) }}");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_body(), "Length: 4");
}

TEST_F(PushNotificationTest, localizedTitleWithInja) {
  supla_push_notification n;
  nlohmann::json d;
  d["str"] = "abcd";

  n.set_localized_title("{{ capitalize(str) }}");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_localized_title(), "Abcd");
}

TEST_F(PushNotificationTest, localizedBodyWithInja) {
  supla_push_notification n;
  nlohmann::json d;
  d["str"] = "abcd";

  n.set_localized_body("{{ upper(str) }}|{{date}}|{{time}}|{{date_time}}");
  n.set_date_time("2023-08-13 13:53:14");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_localized_body(),
            "ABCD|2023-08-13|13:53:14|2023-08-13 13:53:14");
}

TEST_F(PushNotificationTest, injaAndNativeReplacement) {
  supla_push_notification n;
  nlohmann::json d;
  d["str"] = "abcd";

  n.set_localized_body("{{ str }}|{str}");
  n.set_template_data(&d);
  EXPECT_EQ(n.get_localized_body(), "abcd|abcd");
}

TEST_F(PushNotificationTest, injaException) {
  supla_push_notification n;
  n.set_body("{% non_existent_func }}");
  EXPECT_EQ(n.get_body(),
            "[inja.exception.parser_error] (at 1:4) expected statement, got "
            "'non_existent_func'");
}

TEST_F(PushNotificationTest, trim) {
  supla_push_notification n;
  n.set_title(" 	t 	");  // NOLINT
  EXPECT_EQ(n.get_title(), "t");

  n.set_body(" 	b 	");  // NOLINT
  EXPECT_EQ(n.get_body(), "b");

  n.set_localized_title(" 	l t 	");  // NOLINT
  EXPECT_EQ(n.get_localized_title(), "l t");

  n.set_localized_body(" 	l b 	");  // NOLINT
  EXPECT_EQ(n.get_localized_body(), "l b");
}

} /* namespace testing */
