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

#include "ChannelIcValueTest.h"

#include "device/value/channel_ic_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelIcValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_ic_value v1(raw_value);

  EXPECT_EQ(v1.get_ic_value()->counter, 0);

  ((TDS_ImpulseCounter_Value*)raw_value)->counter = 3;

  supla_channel_ic_value v2(raw_value);

  EXPECT_EQ(v2.get_ic_value()->counter, 3);
}

TEST_F(ChannelIcValueTest, icValueConstructor) {
  TDS_ImpulseCounter_Value ic = {};
  ic.counter = 2;

  supla_channel_ic_value v(&ic);
  EXPECT_EQ(v.get_ic_value()->counter, 2);
}

}  // namespace testing
