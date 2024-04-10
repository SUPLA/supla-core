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

#include "RollershutterConfigTest.h"

#include <string>

#include "TestHelper.h"
#include "jsonconfig/channel/roller_shutter_config.h"
#include "proto.h"

using std::string;

namespace testing {

RollershutterConfigTest::RollershutterConfigTest(void) {}

RollershutterConfigTest::~RollershutterConfigTest(void) {}

TEST_F(RollershutterConfigTest, setGetRawConfig) {
  roller_shutter_config config;
  TChannelConfig_Rollershutter raw = {};
  raw.ClosingTimeMS = 123;
  raw.OpeningTimeMS = 345;
  raw.MotorUpsideDown = 1;
  raw.ButtonsUpsideDown = 1;
  raw.TimeMargin = -1;

  config.set_config(&raw);

  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.ClosingTimeMS, 123);
  EXPECT_EQ(raw.OpeningTimeMS, 345);
  EXPECT_EQ(raw.MotorUpsideDown, 1);
  EXPECT_EQ(raw.ButtonsUpsideDown, 1);
  EXPECT_EQ(raw.TimeMargin, -1);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"closingTimeMS\":123,\"openingTimeMS\":345,\"motorUpsideDown\":true,"
      "\"buttonsUpsideDown\":true,\"timeMargin\":-1}");
  free(str);
}

TEST_F(RollershutterConfigTest, buttonsUpsidedown) {
  roller_shutter_config config;
  TChannelConfig_Rollershutter raw = {};

  config.set_config(&raw);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"closingTimeMS\":0,\"openingTimeMS\":0,\"motorUpsideDown\":false,"
      "\"buttonsUpsideDown\":false,\"timeMargin\":0}");
  free(str);

  raw.ButtonsUpsideDown = 1;

  config.set_config(&raw);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"closingTimeMS\":0,\"openingTimeMS\":0,\"motorUpsideDown\":"
               "false,\"timeMargin\":0,\"buttonsUpsideDown\":true}");
  free(str);

  config.set_user_config("{\"buttonsUpsideDown\":false}");
  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.ButtonsUpsideDown, 0);

  config.set_user_config("{\"buttonsUpsideDown\":true}");
  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.ButtonsUpsideDown, 1);
}

TEST_F(RollershutterConfigTest, motorUpsidedown) {
  roller_shutter_config config;
  TChannelConfig_Rollershutter raw = {};

  config.set_config(&raw);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"closingTimeMS\":0,\"openingTimeMS\":0,\"motorUpsideDown\":false,"
      "\"buttonsUpsideDown\":false,\"timeMargin\":0}");
  free(str);

  raw.MotorUpsideDown = 1;

  config.set_config(&raw);

  str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"closingTimeMS\":0,\"openingTimeMS\":0,\"buttonsUpsideDown\":"
               "false,\"timeMargin\":0,\"motorUpsideDown\":true}");
  free(str);

  config.set_user_config("{\"motorUpsideDown\":false}");
  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.MotorUpsideDown, 0);

  config.set_user_config("{\"motorUpsideDown\":true}");
  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.MotorUpsideDown, 1);
}

TEST_F(RollershutterConfigTest, timeMarginEdges) {
  for (int a = -2; a < 102; a++) {
    roller_shutter_config config;
    TChannelConfig_Rollershutter raw = {};
    raw.TimeMargin = a;
    config.set_config(&raw);
    config.get_config(&raw);

    EXPECT_EQ(raw.TimeMargin, a < -1 || a > 100 ? -1 : a);

    string user_config =
        "{\"closingTimeMS\":0,\"openingTimeMS\":0,\"motorUpsideDown\":false,"
        "\"buttonsUpsideDown\":false";

    if (a >= -1 && a <= 100) {
      user_config.append(",\"timeMargin\":");
      user_config.append(std::to_string(a));
    }

    user_config.append("}");

    char *str = config.get_user_config();
    ASSERT_NE(str, nullptr);
    EXPECT_STREQ(str, user_config.c_str());
    bool equal = strncmp(str, user_config.c_str(), user_config.size()) == 0;
    free(str);

    ASSERT_TRUE(equal);
  }
}

TEST_F(RollershutterConfigTest, merge) {
  roller_shutter_config config1;

  config1.set_user_config(
      "{\"a\":\"b\",\"closingTimeMS\":0,\"openingTimeMS\":0,"
      "\"motorUpsideDown\":false,\"buttonsUpsideDown\":false,\"timeMargin\":"
      "105}");

  TChannelConfig_Rollershutter raw = {};
  raw.ClosingTimeMS = 50;
  raw.OpeningTimeMS = 110;
  raw.TimeMargin = -2;

  roller_shutter_config config2;
  config2.set_config(&raw);
  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"a\":\"b\",\"closingTimeMS\":50,\"openingTimeMS\":110,"
      "\"motorUpsideDown\":false,\"buttonsUpsideDown\":false,\"timeMargin\":"
      "105}");

  free(str);
}

} /* namespace testing */
