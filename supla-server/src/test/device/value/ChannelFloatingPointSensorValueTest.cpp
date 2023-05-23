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

#include "ChannelFloatingPointSensorValueTest.h"

#include "device/value/channel_floating_point_sensor_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelFloatingPointSensorValueTest, voidConstructor) {
  supla_channel_floating_point_sensor_value v;
  EXPECT_EQ(v.get_value(), 0.0);

  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  v.get_raw_value(raw_value1);
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelFloatingPointSensorValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_floating_point_sensor_value v1(raw_value);
  EXPECT_EQ(v1.get_value(), 0.0);
}

TEST_F(ChannelFloatingPointSensorValueTest, constructorWithFloatingPointValue) {
  supla_channel_floating_point_sensor_value v1(33.44);
  EXPECT_EQ(v1.get_value(), 33.44);
}

TEST_F(ChannelFloatingPointSensorValueTest, setterAndGetter) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_floating_point_sensor_value v1;
  v1.get_raw_value(raw_value);

  v1.set_value(11.22);
  EXPECT_EQ(v1.get_value(), 11.22);
  v1.get_raw_value(raw_value);

  supla_channel_floating_point_sensor_value v2(raw_value);
  v1.set_value(22.22);
  EXPECT_EQ(v2.get_value(), 11.22);
}

}  // namespace testing
