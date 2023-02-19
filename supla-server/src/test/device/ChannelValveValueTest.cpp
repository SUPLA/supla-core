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

#include <device/ChannelValveValueTest.h>

#include "device/channel_valve_value.h"

namespace testing {

TEST_F(ChannelValveValueTest, voidConstructor) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  TValve_Value vv1 = {};
  TValve_Value vv2 = {};

  supla_channel_valve_value v;
  v.get_raw_value(raw_value2);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
  v.get_valve_value(&vv2);
  EXPECT_EQ(memcmp(&vv1, &vv2, sizeof(TValve_Value)), 0);
  EXPECT_EQ(memcmp(&vv1, v.get_valve_value(), sizeof(TValve_Value)), 0);
}

TEST_F(ChannelValveValueTest, nonVoidConstructor) {
  TValve_Value vv1 = {};
  TValve_Value vv2 = {};

  vv1.closed_percent = 2;
  vv1.flags = 3;

  supla_channel_valve_value v1(&vv1);
  supla_channel_valve_value v2(v1.get_valve_value());

  v2.get_valve_value(&vv2);
  EXPECT_EQ(memcmp(&vv1, &vv2, sizeof(TValve_Value)), 0);
}

TEST_F(ChannelValveValueTest, setterAndGetter) {
  supla_channel_valve_value v;

  TValve_Value vv1 = {};
  TValve_Value vv2 = {};

  vv1.closed = 20;
  vv1.flags = 10;

  v.set_valve_value(&vv1);
  vv1.closed = 0;

  v.get_valve_value(&vv2);

  EXPECT_EQ(vv2.closed, (unsigned char)20);
  EXPECT_EQ(vv2.flags, (unsigned char)10);
}

}  // namespace testing
