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

#include "ChannelDgfValueTest.h"

#include "device/value/channel_dgf_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelDgfValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_dgf_value v1(raw_value);
  EXPECT_EQ(v1.get_dgf_value()->flags, 0);
  EXPECT_EQ(v1.get_dgf_value()->mask, 0);
  EXPECT_EQ(v1.get_dgf_value()->sectionCount, 0);

  ((TDigiglass_Value*)raw_value)->flags = 3;
  ((TDigiglass_Value*)raw_value)->mask = 4;
  ((TDigiglass_Value*)raw_value)->sectionCount = 5;

  supla_channel_dgf_value v2(raw_value);

  EXPECT_EQ(v2.get_dgf_value()->flags, 3);
  EXPECT_EQ(v2.get_dgf_value()->mask, 4);
  EXPECT_EQ(v2.get_dgf_value()->sectionCount, 5);
}

TEST_F(ChannelDgfValueTest, dgfValueConstructor) {
  TDigiglass_Value dgf = {};
  dgf.flags = 1;
  dgf.mask = 2;
  dgf.sectionCount = 3;

  supla_channel_dgf_value v(&dgf);
  EXPECT_EQ(v.get_dgf_value()->flags, 1);
  EXPECT_EQ(v.get_dgf_value()->mask, 2);
  EXPECT_EQ(v.get_dgf_value()->sectionCount, 3);
}

TEST_F(ChannelDgfValueTest, applyChannelProperties) {
  TDigiglass_Value dgf = {};
  dgf.flags = 1;
  dgf.mask = 2;
  dgf.sectionCount = 3;

  supla_channel_dgf_value v(&dgf);
  v.apply_channel_properties(0, 0, 10, 0, 0, 0, nullptr, nullptr);

  EXPECT_EQ(v.get_dgf_value()->flags, 1);
  EXPECT_EQ(v.get_dgf_value()->mask, 2);
  EXPECT_EQ(v.get_dgf_value()->sectionCount, 10);
}

}  // namespace testing
