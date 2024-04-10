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

#include "FacadeBlindConfigTest.h"

#include <string>

#include "TestHelper.h"
#include "jsonconfig/channel/facade_blind_config.h"
#include "proto.h"

using std::string;

namespace testing {

FacadeBlindConfigTest::FacadeBlindConfigTest(void) {}

FacadeBlindConfigTest::~FacadeBlindConfigTest(void) {}

TEST_F(FacadeBlindConfigTest, setGetRawConfig) {
  facade_blind_config config;
  TChannelConfig_FacadeBlind raw = {};
  raw.ClosingTimeMS = 23;
  raw.OpeningTimeMS = 45;
  raw.MotorUpsideDown = 2;
  raw.ButtonsUpsideDown = 3;
  raw.TimeMargin = 5;
  raw.TiltingTimeMS = 789;
  raw.Tilt0Angle = 8;
  raw.Tilt100Angle = 9;
  raw.FacadeBlindType = SUPLA_FACADEBLIND_TYPE_TILTS_ONLY_WHEN_FULLY_CLOSED;

  config.set_config(&raw);

  raw = {};
  config.get_config(&raw);

  EXPECT_EQ(raw.ClosingTimeMS, 23);
  EXPECT_EQ(raw.OpeningTimeMS, 45);
  EXPECT_EQ(raw.MotorUpsideDown, 1);
  EXPECT_EQ(raw.ButtonsUpsideDown, 1);
  EXPECT_EQ(raw.TimeMargin, 5);
  EXPECT_EQ(raw.TiltingTimeMS, 789);
  EXPECT_EQ(raw.Tilt0Angle, 8);
  EXPECT_EQ(raw.Tilt100Angle, 9);
  EXPECT_EQ(raw.FacadeBlindType,
            SUPLA_FACADEBLIND_TYPE_TILTS_ONLY_WHEN_FULLY_CLOSED);

  char *str = config.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(str,
               "{\"closingTimeMS\":23,\"openingTimeMS\":45,\"motorUpsideDown\":"
               "true,\"buttonsUpsideDown\":true,\"timeMargin\":5,"
               "\"tiltingTimeMS\":789,\"tilt0Angle\":8,\"tilt100Angle\":9,"
               "\"fasadeBlindType\":\"TILTS_ONLY_WHEN_FULLY_CLOSED\"}");
  free(str);
}

TEST_F(FacadeBlindConfigTest, tilt0AngleEdges) {
  for (int a = -2; a < 362; a++) {
    facade_blind_config config;
    TChannelConfig_FacadeBlind raw = {};
    raw.Tilt0Angle = a;
    config.set_config(&raw);
    config.get_config(&raw);

    EXPECT_EQ(raw.Tilt0Angle, a < 0 || a > 360 ? 0 : a);

    string user_config =
        "{\"closingTimeMS\":0,\"openingTimeMS\":0,\"motorUpsideDown\":false,"
        "\"buttonsUpsideDown\":false,\"timeMargin\":0,\"tiltingTimeMS\":0";

    if (a >= 0 && a <= 360) {
      user_config.append(",\"tilt0Angle\":");
      user_config.append(std::to_string(a));
    }

    user_config.append(",\"tilt100Angle\":0,\"fasadeBlindType\":\"UNKNOWN\"}");

    char *str = config.get_user_config();
    ASSERT_NE(str, nullptr);
    EXPECT_STREQ(str, user_config.c_str());
    bool equal = strncmp(str, user_config.c_str(), user_config.size()) == 0;
    free(str);

    ASSERT_TRUE(equal);
  }
}

TEST_F(FacadeBlindConfigTest, merge) {
  facade_blind_config config1;

  config1.set_user_config(
      "{\"a\":\"b\",\"closingTimeMS\":0,\"openingTimeMS\":0,"
      "\"motorUpsideDown\":false,\"buttonsUpsideDown\":false,\"timeMargin\":0,"
      "\"tiltingTimeMS\":0,\"tilt0Angle\":0,\"tilt100Angle\":0,"
      "\"fasadeBlindType\":\"UNKNOWN\"}");

  TChannelConfig_FacadeBlind raw = {};
  raw.FacadeBlindType = SUPLA_FACADEBLIND_TYPE_TILTS_ONLY_WHEN_FULLY_CLOSED;

  facade_blind_config config2;
  config2.set_config(&raw);
  config2.merge(&config1);

  char *str = config1.get_user_config();
  ASSERT_NE(str, nullptr);
  EXPECT_STREQ(
      str,
      "{\"a\":\"b\",\"closingTimeMS\":0,\"openingTimeMS\":0,"
      "\"motorUpsideDown\":false,\"buttonsUpsideDown\":false,\"timeMargin\":0,"
      "\"tiltingTimeMS\":0,\"tilt0Angle\":0,\"tilt100Angle\":0,"
      "\"fasadeBlindType\":\"TILTS_ONLY_WHEN_FULLY_CLOSED\"}");

  free(str);
}

} /* namespace testing */
