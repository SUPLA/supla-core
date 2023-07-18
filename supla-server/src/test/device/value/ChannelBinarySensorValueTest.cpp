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

#include <device/value/ChannelBinarySensorValueTest.h>

#include "device/value/channel_binary_sensor_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelBinarySensorValueTest, voidConstructor) {
  supla_channel_binary_sensor_value v;
  EXPECT_FALSE(v.is_hi());

  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  v.get_raw_value(raw_value1);
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelBinarySensorValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_binary_sensor_value v1(raw_value);
  EXPECT_FALSE(v1.is_hi());

  raw_value[0] = 1;
  supla_channel_binary_sensor_value v2(raw_value);
  EXPECT_TRUE(v2.is_hi());
}

TEST_F(ChannelBinarySensorValueTest, setterAndGetter) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_binary_sensor_value v;
  v.get_raw_value(raw_value);
  EXPECT_EQ(raw_value[0], 0);
  EXPECT_FALSE(v.is_hi());
  v.set_hi(true);
  v.get_raw_value(raw_value);
  EXPECT_EQ(raw_value[0], 1);
}

TEST_F(ChannelBinarySensorValueTest, applyChannelProperties) {
  supla_channel_binary_sensor_value v;
  v.set_hi(true);
  v.apply_channel_properties(SUPLA_CHANNELTYPE_SENSORNC, 0, 0, 0, 0, 0,
                             nullptr);
  EXPECT_FALSE(v.is_hi());

  v.set_hi(true);
  v.apply_channel_properties(SUPLA_CHANNELTYPE_SENSORNC, 0, 0, 0, 1, 0,
                             nullptr);
  EXPECT_TRUE(v.is_hi());

  v.set_hi(true);
  v.apply_channel_properties(0, 0, 0, 0, 1, 0, nullptr);
  EXPECT_FALSE(v.is_hi());
}

}  // namespace testing
