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

#include "device/value/ChannelRgbwValueTest.h"

#include <string>

#include "device/value/channel_rgbw_value.h"
#include "devicechannel.h"  // NOLINT
#include "proto.h"

namespace testing {

TEST_F(ChannelRgbwValueTest, voidConstructor) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};
  TRGBW_Value rgbw1 = {};
  TRGBW_Value rgbw2 = {};

  supla_channel_rgbw_value v(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING);
  v.get_raw_value(raw_value1);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);

  v.get_rgbw(&rgbw1);

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TRGBW_Value)), 0);
}

TEST_F(ChannelRgbwValueTest, nonVoidConstructors) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};
  TRGBW_Value rgbw1 = {};
  TRGBW_Value rgbw2 = {};

  rgbw1.R = 0xAA;
  rgbw1.G = 0xBB;
  rgbw1.B = 0xCC;
  rgbw1.brightness = 10;
  rgbw1.colorBrightness = 20;
  rgbw1.onOff = 1;
  supla_channel_rgbw_value v1(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, &rgbw1);
  v1.get_rgbw(&rgbw2);
  v1.get_raw_value(raw_value2);

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TRGBW_Value)), 0);
  EXPECT_NE(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);

  memset(&rgbw1, 0, sizeof(TRGBW_Value));
  memset(raw_value1, 0, sizeof(raw_value1));

  supla_channel_rgbw_value v2(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING,
                              raw_value2);
  v2.get_rgbw(&rgbw1);
  v2.get_raw_value(raw_value1);

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TRGBW_Value)), 0);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);

  memset(&rgbw2, 0, sizeof(TRGBW_Value));
  memset(raw_value2, 0, sizeof(raw_value2));

  supla_channel_rgbw_value v3(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, &rgbw1);
  v2.get_rgbw(&rgbw2);
  v2.get_raw_value(raw_value2);

  EXPECT_EQ(memcmp(&rgbw1, &rgbw2, sizeof(TRGBW_Value)), 0);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelRgbwValueTest, gettersAndSetters) {
  TRGBW_Value rgbw1 = {};
  TRGBW_Value rgbw2 = {};

  rgbw1.R = 0xAA;
  rgbw1.G = 0xBB;
  rgbw1.B = 0xCC;
  rgbw1.brightness = 10;
  rgbw1.colorBrightness = 20;
  rgbw1.onOff = 1;
  rgbw1.whiteTemperature = 5;
  supla_channel_rgbw_value v(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, &rgbw1);

  int color = 0;
  char color_brightness = 0;
  char brightness = 0;
  char white_temperature = 0;

  v.get_rgbw(&color, &color_brightness, &brightness, &white_temperature);
  v.get_rgbw(&rgbw2);

  EXPECT_EQ(brightness, 10);
  EXPECT_EQ(white_temperature, 5);
  EXPECT_EQ(color_brightness, 20);
  EXPECT_EQ(color, 0xAABBCC);

  EXPECT_EQ(v.get_brightness(), 10);
  EXPECT_EQ(v.get_white_temperature(), 5);
  EXPECT_EQ(v.get_color_brightness(), 20);
  EXPECT_EQ(v.get_color(), 0xAABBCC);

  EXPECT_EQ(rgbw2.R, 0xAA);
  EXPECT_EQ(rgbw2.G, 0xBB);
  EXPECT_EQ(rgbw2.B, 0xCC);
  EXPECT_EQ(rgbw2.brightness, 10);
  EXPECT_EQ(rgbw2.colorBrightness, 20);
  EXPECT_EQ(rgbw2.onOff, 1);

  v.set_brightness(40);
  v.set_white_temperature(15);
  v.set_color_brightness(60);
  v.set_color(0x112233);

  v.get_rgbw(&color, &color_brightness, &brightness, &white_temperature);
  v.get_rgbw(&rgbw2);

  EXPECT_EQ(brightness, 40);
  EXPECT_EQ(white_temperature, 15);
  EXPECT_EQ(color_brightness, 60);
  EXPECT_EQ(color, 0x112233);

  EXPECT_EQ(v.get_brightness(), 40);
  EXPECT_EQ(v.get_white_temperature(), 15);
  EXPECT_EQ(v.get_color_brightness(), 60);
  EXPECT_EQ(v.get_color(), 0x112233);

  EXPECT_EQ(rgbw2.R, (char)0x11);
  EXPECT_EQ(rgbw2.G, (char)0x22);
  EXPECT_EQ(rgbw2.B, (char)0x33);
  EXPECT_EQ(rgbw2.brightness, 40);
  EXPECT_EQ(rgbw2.whiteTemperature, 15);
  EXPECT_EQ(rgbw2.colorBrightness, 60);
  EXPECT_EQ(rgbw2.onOff, 1);

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  v.get_raw_value(raw_value);

  supla_channel_rgbw_value v2(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, raw_value);
  EXPECT_EQ(v2.get_brightness(), 40);
  EXPECT_EQ(v2.get_color_brightness(), 60);
  EXPECT_EQ(v2.get_color(), 0x112233);
}

TEST_F(ChannelRgbwValueTest, getVbtValue) {
  supla_channel_rgbw_value value(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING);

  double vbt_value = 0;
  EXPECT_FALSE(value.get_vbt_value(var_name_none, &vbt_value));

  value.set_color(1234);
  value.set_brightness(21);
  value.set_color_brightness(35);

  EXPECT_TRUE(value.get_vbt_value(var_name_color, &vbt_value));
  EXPECT_EQ(vbt_value, 1234);

  EXPECT_TRUE(value.get_vbt_value(var_name_brightness, &vbt_value));
  EXPECT_EQ(vbt_value, 21);

  EXPECT_TRUE(value.get_vbt_value(var_name_color_brightness, &vbt_value));
  EXPECT_EQ(vbt_value, 35);
}
TEST_F(ChannelRgbwValueTest, templateData) {
  supla_channel_rgbw_value value(SUPLA_CHANNELFNC_RGBLIGHTING);
  value.set_color(0xAABBCC);
  value.set_color_brightness(25);

  auto m = value.get_template_data();
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m["color"].get<std::string>(), "0xAABBCC");
  EXPECT_EQ(m["color_brightness"].get<int>(), 25);
  EXPECT_TRUE(m["on"].get<bool>());

  value = supla_channel_rgbw_value(SUPLA_CHANNELFNC_DIMMER);
  value.set_brightness(17);

  m = value.get_template_data();
  EXPECT_EQ(m.size(), 2);
  EXPECT_EQ(m["brightness"].get<int>(), 17);
  EXPECT_TRUE(m["on"].get<bool>());

  value = supla_channel_rgbw_value(SUPLA_CHANNELFNC_DIMMER_CCT);
  value.set_brightness(17);
  value.set_white_temperature(20);

  m = value.get_template_data();
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m["brightness"].get<int>(), 17);
  EXPECT_EQ(m["white_temperature"].get<int>(), 20);
  EXPECT_TRUE(m["on"].get<bool>());

  value = supla_channel_rgbw_value(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING);
  value.set_color(0xBBAACC);
  value.set_color_brightness(50);
  value.set_brightness(100);

  m = value.get_template_data();
  EXPECT_EQ(m.size(), 4);
  EXPECT_EQ(m["brightness"].get<int>(), 100);
  EXPECT_EQ(m["color"].get<std::string>(), "0xBBAACC");
  EXPECT_EQ(m["color_brightness"].get<int>(), 50);
  EXPECT_TRUE(m["on"].get<bool>());

  value = supla_channel_rgbw_value(SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB);
  value.set_color(0xBBAACC);
  value.set_color_brightness(50);
  value.set_brightness(100);
  value.set_white_temperature(90);

  m = value.get_template_data();
  EXPECT_EQ(m.size(), 5);
  EXPECT_EQ(m["brightness"].get<int>(), 100);
  EXPECT_EQ(m["color"].get<std::string>(), "0xBBAACC");
  EXPECT_EQ(m["color_brightness"].get<int>(), 50);
  EXPECT_EQ(m["white_temperature"].get<int>(), 90);
  EXPECT_TRUE(m["on"].get<bool>());
}

}  // namespace testing
