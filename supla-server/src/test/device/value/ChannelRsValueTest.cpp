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

#include "ChannelRsValueTest.h"

#include "device/value/channel_rs_value.h"

namespace testing {

TEST_F(ChannelRsValueTest, setterAndGetter) {
  TDSC_RollerShutterValue value = {};

  value.bottom_position = 1;
  value.flags = 2;
  value.position = 3;

  supla_channel_rs_value v1(&value);
  EXPECT_EQ(memcmp(v1.get_rs_value(), &value, sizeof(TDSC_RollerShutterValue)),
            0);

  supla_channel_rs_value v2(v1.get_rs_value());

  EXPECT_EQ(memcmp(v2.get_rs_value(), v1.get_rs_value(),
                   sizeof(TDSC_RollerShutterValue)),
            0);

  value.position = 50;
  v2.set_rs_value(&value);
  value.position = 100;

  EXPECT_EQ(v2.get_rs_value()->position, 50);
}

TEST_F(ChannelRsValueTest, applyChannelProperties) {
  TDSC_RollerShutterValue value = {};

  value.bottom_position = 1;
  value.flags = 2;
  value.position = 3;

  supla_channel_rs_value v(&value);
  v.apply_channel_properties(0, 0, 0, 0, 0, 10, nullptr);

  EXPECT_EQ(v.get_rs_value()->bottom_position, 10);
  EXPECT_EQ(v.get_rs_value()->flags, 2);
  EXPECT_EQ(v.get_rs_value()->position, 3);
}

}  // namespace testing
