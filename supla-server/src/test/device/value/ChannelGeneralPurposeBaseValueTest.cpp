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

#include "ChannelGeneralPurposeBaseValueTest.h"

#include "device/value/channel_general_purpose_base_value.h"

namespace testing {

TEST_F(ChannelGeneralPurposeBaseValueTest, defaultConstructor) {
  supla_channel_general_purpose_base_value val1;
  EXPECT_EQ(val1.get_value(), 0.0);

  const char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_general_purpose_base_value val2(raw_value);
  EXPECT_EQ(val2.get_value(), 0.0);
}

TEST_F(ChannelGeneralPurposeBaseValueTest, setAndGet) {
  supla_channel_general_purpose_base_value val;
  val.set_value(12.34);
  EXPECT_DOUBLE_EQ(val.get_value(), 12.34);
}

TEST_F(ChannelGeneralPurposeBaseValueTest, applyChannelProperties) {
  supla_channel_general_purpose_base_value val;
  val.set_value(12.34);

  supla_json_config config;
  config.set_user_config(
      "{\"valueDivider\":2000,\"valueMultiplier\":4000,\"valueAdded\":5000}");

  val.apply_channel_properties(0, 0, 0, 0, 0, 0, &config);
  EXPECT_DOUBLE_EQ(val.get_value(), 29.68);
}

TEST_F(ChannelGeneralPurposeBaseValueTest, getVbtValue) {
  supla_channel_general_purpose_base_value value;
  value.set_value(12.34);

  double vbt_value = 0;
  EXPECT_TRUE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 12.34);
}

}  // namespace testing
