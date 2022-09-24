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

#include "ActionConfigTest.h"

namespace testing {

ActionConfigTest::ActionConfigTest(void) {}

ActionConfigTest::~ActionConfigTest(void) {}

TEST_F(ActionConfigTest, actionId) {
  EXPECT_EQ(config.get_action_id(), 0);
  config.set_action_id(ACTION_SHUT);
  EXPECT_EQ(config.get_action_id(), ACTION_SHUT);
}

TEST_F(ActionConfigTest, subjectType) {
  EXPECT_EQ(config.get_subject_type(), stUnknown);
  config.set_subject_type(stChannel);
  EXPECT_EQ(config.get_subject_type(), stChannel);
}

TEST_F(ActionConfigTest, subjectId) {
  EXPECT_EQ(config.get_subject_id(), 0);
  config.set_subject_id(15);
  EXPECT_EQ(config.get_subject_id(), 15);
}

TEST_F(ActionConfigTest, sourceDeviceId) {
  EXPECT_EQ(config.get_source_device_id(), 0);
  config.set_source_device_id(155);
  EXPECT_EQ(config.get_source_device_id(), 155);
}

TEST_F(ActionConfigTest, sourceChannelId) {
  EXPECT_EQ(config.get_source_channel_id(), 0);
  config.set_source_channel_id(125);
  EXPECT_EQ(config.get_source_channel_id(), 125);
}

TEST_F(ActionConfigTest, percentage) {
  EXPECT_EQ(config.get_percentage(), 0);
  config.set_percentage(22);
  EXPECT_EQ(config.get_percentage(), 22);
}

TEST_F(ActionConfigTest, rgbw) {
  TAction_RGBW_Parameters rgbw1 = {};
  TAction_RGBW_Parameters rgbw2 = config.get_rgbw();

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TAction_RGBW_Parameters)), 0);

  rgbw1.Brightness = 10;
  rgbw1.Color = 20;
  rgbw1.ColorBrightness = 30;
  rgbw1.ColorRandom = false;
  rgbw1.OnOff = true;

  config.set_rgbw(rgbw1);
  rgbw2 = config.get_rgbw();

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TAction_RGBW_Parameters)), 0);
}

TEST_F(ActionConfigTest, jsonPercentage) {
  EXPECT_EQ(config.get_percentage(), 0);

  config.apply_json_params("{\"percentage\":45}");
  EXPECT_EQ(config.get_percentage(), 45);

  config.apply_json_params("{\"perCentaGe\":80}");
  EXPECT_EQ(config.get_percentage(), 80);

  config.apply_json_params("{\"percentage\":110}");
  EXPECT_EQ(config.get_percentage(), 100);

  config.apply_json_params("{\"percentage\":-1}");
  EXPECT_EQ(config.get_percentage(), 0);
}

TEST_F(ActionConfigTest, jsonSourceChannelId) {
  EXPECT_EQ(config.get_source_channel_id(), 0);
  config.apply_json_params("{\"sourceChannelId\":46865}");
  EXPECT_EQ(config.get_source_channel_id(), 46865);
}

TEST_F(ActionConfigTest, jsonSourceDeviceId) {
  EXPECT_EQ(config.get_source_device_id(), 0);
  config.apply_json_params("{\"sourceDeviceId\":1231}");
  EXPECT_EQ(config.get_source_device_id(), 1231);
}

TEST_F(ActionConfigTest, jsonRGBW) {
  TAction_RGBW_Parameters rgbw1 = {};
  TAction_RGBW_Parameters rgbw2 = config.get_rgbw();

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TAction_RGBW_Parameters)), 0);

  config.apply_json_params(
      "{\"brightness\":32,\"hue\":244,\"color_brightness\":22}");

  rgbw2 = config.get_rgbw();
  EXPECT_EQ(rgbw2.Brightness, 32);
  EXPECT_EQ(rgbw2.Color, (unsigned int)0x1000FF);
  EXPECT_EQ(rgbw2.ColorBrightness, 22);
  EXPECT_FALSE(rgbw2.ColorRandom);
}

TEST_F(ActionConfigTest, jsonRGBW_white) {
  TAction_RGBW_Parameters rgbw1 = {};
  TAction_RGBW_Parameters rgbw2 = config.get_rgbw();

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TAction_RGBW_Parameters)), 0);

  config.apply_json_params(
      "{\"brightness\":32,\"hue\":\"white\",\"color_brightness\":22}");

  rgbw2 = config.get_rgbw();
  EXPECT_EQ(rgbw2.Brightness, 32);
  EXPECT_EQ(rgbw2.Color, (unsigned int)0xFFFFFF);
  EXPECT_EQ(rgbw2.ColorBrightness, 22);
  EXPECT_FALSE(rgbw2.ColorRandom);
}

TEST_F(ActionConfigTest, jsonRGBW_random) {
  TAction_RGBW_Parameters rgbw1 = {};
  TAction_RGBW_Parameters rgbw2 = config.get_rgbw();

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TAction_RGBW_Parameters)), 0);

  config.apply_json_params(
      "{\"brightness\":32,\"hue\":\"random\",\"color_brightness\":22}");

  rgbw2 = config.get_rgbw();
  EXPECT_EQ(rgbw2.Brightness, 32);
  EXPECT_GT(rgbw2.Color, (unsigned int)0);
  EXPECT_EQ(rgbw2.ColorBrightness, 22);
  EXPECT_TRUE(rgbw2.ColorRandom);

  unsigned int color = 0;

  for (short n = 0; n < 10; n++) {
    color = rgbw2.Color;

    short a = 0;
    for (a = 0; a < 10; a++) {
      rgbw2 = config.get_rgbw();
      usleep(1000);
      if (rgbw2.Color != color) {
        break;
      }
    }

    EXPECT_LT(a, 10);
  }
}

TEST_F(ActionConfigTest, jsonMultipleParams) {
  EXPECT_EQ(config.get_percentage(), 0);
  config.apply_json_params("{\"percentage\":23}");
  EXPECT_EQ(config.get_percentage(), 23);

  EXPECT_EQ(config.get_source_channel_id(), 0);
  config.apply_json_params("{\"sourceChannelId\":555666}");
  EXPECT_EQ(config.get_source_channel_id(), 555666);

  EXPECT_EQ(config.get_source_device_id(), 0);
  config.apply_json_params("{\"sourceDeviceId\":1333}");
  EXPECT_EQ(config.get_source_device_id(), 1333);

  TAction_RGBW_Parameters rgbw1 = {};
  TAction_RGBW_Parameters rgbw2 = config.get_rgbw();

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TAction_RGBW_Parameters)), 0);

  config.apply_json_params(
      "{\"brightness\":12,\"hue\":244,\"color_brightness\":23}");

  rgbw2 = config.get_rgbw();
  EXPECT_EQ(rgbw2.Brightness, 12);
  EXPECT_EQ(rgbw2.Color, (unsigned int)0x1000FF);
  EXPECT_EQ(rgbw2.ColorBrightness, 23);
  EXPECT_FALSE(rgbw2.ColorRandom);
}

} /* namespace testing */
