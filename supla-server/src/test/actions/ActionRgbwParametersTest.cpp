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

#include "ActionRgbwParametersTest.h"

#include "actions/action_rgbw_parameters.h"

namespace testing {

ActionRgbwParametersTest::ActionRgbwParametersTest(void) {}

ActionRgbwParametersTest::~ActionRgbwParametersTest(void) {}

TEST_F(ActionRgbwParametersTest, setParams) {
  supla_action_rgbw_parameters params;
  EXPECT_TRUE(params.set_params(0xFFAAFF, false, 1, 10, 2, 3, 4));
  EXPECT_EQ(params.get_rgbw().Color, 0xFFAAFF);
  EXPECT_EQ(params.get_rgbw().ColorRandom, 0);
  EXPECT_EQ(params.get_rgbw().ColorBrightness, 1);
  EXPECT_EQ(params.get_rgbw().Brightness, 10);
  EXPECT_EQ(params.get_rgbw().OnOff, 2);
  EXPECT_EQ(params.get_rgbw().Command, 3);
  EXPECT_EQ(params.get_rgbw().WhiteTemperature, 4);

  EXPECT_TRUE(params.set_params(0xAABBCC, true, 0, 0, 0, 0, 0));
  EXPECT_EQ(params.get_rgbw().ColorRandom, 1);

  EXPECT_TRUE(params.set_params(0, false, 0, 0, 0, 0, 0));
  EXPECT_EQ(params.get_rgbw().Color, 0);
  EXPECT_EQ(params.get_rgbw().ColorRandom, 0);
  EXPECT_EQ(params.get_rgbw().ColorBrightness, 0);
  EXPECT_EQ(params.get_rgbw().Brightness, 0);
  EXPECT_EQ(params.get_rgbw().OnOff, 0);
  EXPECT_EQ(params.get_rgbw().Command, 0);
  EXPECT_EQ(params.get_rgbw().WhiteTemperature, 0);

  EXPECT_FALSE(params.set_params(0, false, -2, 0, 0, 0, 0));
  EXPECT_FALSE(params.set_params(0, false, 101, 0, 0, 0, 0));
  EXPECT_FALSE(params.set_params(0, false, 0, -2, 0, 0, 0));
  EXPECT_FALSE(params.set_params(0, false, 0, 101, -2, 0, 0));
  EXPECT_FALSE(params.set_params(0, false, 0, 0, 4, 0, 0));
  EXPECT_FALSE(params.set_params(0, false, 0, 0, 0, -2, 0));
  EXPECT_FALSE(params.set_params(
      0, false, 0, 0, 0, RGBW_COMMAND_SET_WHITE_TEMPERATURE_WITHOUT_TURN_ON + 1,
      0));
  EXPECT_FALSE(params.set_params(0, false, 0, 0, 0, 0, -2));
  EXPECT_FALSE(params.set_params(0, false, 0, 0, 0, 0, 101));
}

} /* namespace testing */
