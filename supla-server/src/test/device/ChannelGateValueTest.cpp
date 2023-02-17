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

#include <device/ChannelGateValueTest.h>

#include "device/channel_gate_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelGateValueTest, voidConstructor) {
  supla_channel_gate_value v;
  EXPECT_FALSE(v.is_relay_hi());
  EXPECT_EQ(v.get_opening_sensor_level(), gsl_unknown);
  EXPECT_EQ(v.get_partial_opening_sensor_level(), gsl_unknown);
}

TEST_F(ChannelGateValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_gate_value v1(raw_value);
  EXPECT_FALSE(v1.is_relay_hi());
  EXPECT_EQ(v1.get_opening_sensor_level(), gsl_unknown);
  EXPECT_EQ(v1.get_partial_opening_sensor_level(), gsl_unknown);

  raw_value[0] = 1;
  supla_channel_gate_value v2(raw_value);
  EXPECT_TRUE(v2.is_relay_hi());
  EXPECT_EQ(v2.get_opening_sensor_level(), gsl_unknown);
  EXPECT_EQ(v2.get_partial_opening_sensor_level(), gsl_unknown);

  memset(raw_value, 1, SUPLA_CHANNELVALUE_SIZE);
  supla_channel_gate_value v3(raw_value);
  EXPECT_TRUE(v3.is_relay_hi());
  EXPECT_EQ(v3.get_opening_sensor_level(), gsl_unknown);
  EXPECT_EQ(v3.get_partial_opening_sensor_level(), gsl_unknown);
}

TEST_F(ChannelGateValueTest, openingSensorConstructor) {
  supla_channel_gate_value v1(gsl_open, gsl_closed);
  EXPECT_FALSE(v1.is_relay_hi());
  EXPECT_EQ(v1.get_opening_sensor_level(), gsl_open);
  EXPECT_EQ(v1.get_partial_opening_sensor_level(), gsl_closed);
}

TEST_F(ChannelGateValueTest, openingSensorGetterSetter) {
  supla_channel_gate_value v;
  EXPECT_EQ(v.get_opening_sensor_level(), gsl_unknown);
  v.set_opening_sensor_level(gsl_closed);
  EXPECT_EQ(v.get_opening_sensor_level(), gsl_closed);
}

TEST_F(ChannelGateValueTest, partialOpeningSensorGetterSetter) {
  supla_channel_gate_value v;
  EXPECT_EQ(v.get_partial_opening_sensor_level(), gsl_unknown);
  v.set_partial_opening_sensor_level(gsl_closed);
  EXPECT_EQ(v.get_partial_opening_sensor_level(), gsl_closed);
}

}  // namespace testing
