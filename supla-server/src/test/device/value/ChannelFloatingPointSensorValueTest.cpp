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
#include "proto.h"

namespace testing {

TEST_F(ChannelFloatingPointSensorValueTest, voidConstructor) {
  supla_channel_floating_point_sensor_value v(SUPLA_CHANNELFNC_WEIGHTSENSOR);
  EXPECT_EQ(v.get_value(), 0.0);

  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  v.get_raw_value(raw_value1);
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelFloatingPointSensorValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_floating_point_sensor_value v1(SUPLA_CHANNELFNC_DEPTHSENSOR,
                                               raw_value);
  EXPECT_EQ(v1.get_value(), 0.0);
}

TEST_F(ChannelFloatingPointSensorValueTest, constructorWithFloatingPointValue) {
  supla_channel_floating_point_sensor_value v1(SUPLA_CHANNELFNC_DISTANCESENSOR,
                                               33.44);
  EXPECT_EQ(v1.get_value(), 33.44);
}

TEST_F(ChannelFloatingPointSensorValueTest, setterAndGetter) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_floating_point_sensor_value v1(SUPLA_CHANNELFNC_WINDSENSOR);
  v1.get_raw_value(raw_value);

  v1.set_value(11.22);
  EXPECT_EQ(v1.get_value(), 11.22);
  v1.get_raw_value(raw_value);

  supla_channel_floating_point_sensor_value v2(SUPLA_CHANNELFNC_WINDSENSOR,
                                               raw_value);
  v1.set_value(22.22);
  EXPECT_EQ(v2.get_value(), 11.22);
}

TEST_F(ChannelFloatingPointSensorValueTest, getVbtValue) {
  supla_channel_floating_point_sensor_value value(
      SUPLA_CHANNELFNC_PRESSURESENSOR);
  value.set_value(11.22);

  double vbt_value = 0;

  EXPECT_TRUE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 11.22);
}

TEST_F(ChannelFloatingPointSensorValueTest, getVbtValue_Disatnce) {
  supla_channel_floating_point_sensor_value value(
      SUPLA_CHANNELFNC_DISTANCESENSOR);
  value.set_value(-1);

  double vbt_value = 5;

  EXPECT_FALSE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 5);

  value.set_value(100);

  vbt_value = 100;

  EXPECT_TRUE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 100);
}

TEST_F(ChannelFloatingPointSensorValueTest, templateData) {
  supla_channel_floating_point_sensor_value value(SUPLA_CHANNELFNC_RAINSENSOR);
  value.set_value(11.22);

  auto m = value.get_template_data();
  EXPECT_EQ(m.size(), 1);
  EXPECT_EQ(m["value"].get<double>(), 11.22);
}

}  // namespace testing
