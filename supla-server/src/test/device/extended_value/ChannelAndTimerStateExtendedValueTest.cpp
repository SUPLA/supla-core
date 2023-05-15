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

#include "ChannelAndTimerStateExtendedValueTest.h"

#include "device/extended_value/channel_and_timer_state_extended_value.h"

namespace testing {

TEST_F(ChannelAndTimerStateExtendedValueTest, nulls) {
  supla_channel_and_timer_state_extended_value v(nullptr, nullptr);

  TChannelAndTimerState_ExtendedValue raw_state = {};
  TSuplaChannelExtendedValue raw_ev = {};

  EXPECT_FALSE(v.get_raw_value(&raw_state));
  EXPECT_FALSE(v.get_raw_value(&raw_ev));
}

TEST_F(ChannelAndTimerStateExtendedValueTest, empty) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  supla_channel_and_timer_state_extended_value v(&ev_raw1, nullptr);

  TChannelAndTimerState_ExtendedValue raw_state = {};
  TSuplaChannelExtendedValue raw_ev = {};

  EXPECT_FALSE(v.get_raw_value(&raw_state));
  EXPECT_FALSE(v.get_raw_value(&raw_ev));
}

TEST_F(ChannelAndTimerStateExtendedValueTest, randomData) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TChannelAndTimerState_ExtendedValue);
  ev_raw1.type = EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned int seed = tv.tv_sec + tv.tv_usec;
  for (unsigned int a = 0; a < ev_raw1.size; a++) {
    ev_raw1.value[a] = rand_r(&seed);
  }

  ((TChannelAndTimerState_ExtendedValue*)ev_raw1.value)->Timer.SenderNameSize =
      0;
  ((TChannelAndTimerState_ExtendedValue*)ev_raw1.value)->Timer.RemainingTimeMs =
      0;

  supla_channel_and_timer_state_extended_value v(&ev_raw1, nullptr);
  EXPECT_EQ(v.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                   SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                   sizeof(TChannelAndTimerState_ExtendedValue) -
                                   SUPLA_SENDER_NAME_MAXSIZE);
  EXPECT_EQ(v.get_value_size(), sizeof(TChannelAndTimerState_ExtendedValue) -
                                    SUPLA_SENDER_NAME_MAXSIZE);

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_TRUE(v.get_raw_value(&ev_raw2));
  EXPECT_TRUE(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)));

  TChannelAndTimerState_ExtendedValue state_raw = {};
  EXPECT_TRUE(v.get_raw_value(&state_raw));

  EXPECT_EQ(memcmp(&state_raw, ev_raw2.value, v.get_value_size()), 0);
}

TEST_F(ChannelAndTimerStateExtendedValueTest, countdownEndsAt) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TChannelAndTimerState_ExtendedValue);
  ev_raw1.type = EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;

  ((TChannelAndTimerState_ExtendedValue*)ev_raw1.value)->Timer.RemainingTimeMs =
      5000;

  supla_channel_and_timer_state_extended_value v(&ev_raw1, nullptr);

  TChannelAndTimerState_ExtendedValue state_raw = {};
  EXPECT_TRUE(v.get_raw_value(&state_raw));

  struct timeval now;
  gettimeofday(&now, nullptr);

  EXPECT_GE(state_raw.Timer.CountdownEndsAt, now.tv_sec + 5);
  EXPECT_LE(state_raw.Timer.CountdownEndsAt, now.tv_sec + 6);
}

}  // namespace testing
