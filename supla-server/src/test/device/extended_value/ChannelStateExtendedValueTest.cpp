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

#include "ChannelStateExtendedValueTest.h"

#include "device/extended_value/channel_state_extended_value.h"

namespace testing {

TEST_F(ChannelStateExtendedValueTest, nulls) {
  supla_channel_state_extended_value ev(nullptr);

  TSuplaChannelExtendedValue ev_raw = {};
  EXPECT_FALSE(ev.get_raw_value(&ev_raw));

  TChannelState_ExtendedValue ts = {};
  EXPECT_FALSE(ev.get_raw_value(&ts));
}

TEST_F(ChannelStateExtendedValueTest, empty) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  supla_channel_state_extended_value ev(&ev_raw1);

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_FALSE(ev.get_raw_value(&ev_raw2));

  TChannelState_ExtendedValue ts = {};
  EXPECT_FALSE(ev.get_raw_value(&ts));
}

TEST_F(ChannelStateExtendedValueTest, randomData) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.type = EV_TYPE_CHANNEL_STATE_V1;
  ev_raw1.size = sizeof(TChannelState_ExtendedValue);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned int seed = tv.tv_sec + tv.tv_usec;
  for (unsigned int a = 0; a < ev_raw1.size; a++) {
    ev_raw1.value[a] = rand_r(&seed);
  }

  supla_channel_state_extended_value ev(&ev_raw1);

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_EQ(ev.get_value_size(), sizeof(TChannelState_ExtendedValue));
  EXPECT_EQ(ev.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                    SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                    sizeof(TChannelState_ExtendedValue));
  EXPECT_TRUE(ev.get_raw_value(&ev_raw2));
  EXPECT_EQ(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)), 0);
}

}  // namespace testing
