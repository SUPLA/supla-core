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

#include "TestHelper.h"
#include "device/extended_value/channel_and_timer_state_extended_value.h"
#include "log.h"

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

TEST_F(ChannelAndTimerStateExtendedValueTest, defaultConstructor) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  TSuplaChannelExtendedValue ev_raw2 = {};

  ev_raw1.size = sizeof(TChannelAndTimerState_ExtendedValue);

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  ((TChannelAndTimerState_ExtendedValue *)ev_raw1.value)->Timer.SenderNameSize =
      sizeof(((TChannelAndTimerState_ExtendedValue *)ev_raw1.value)
                 ->Timer.SenderName);

  {
    supla_channel_and_timer_state_extended_value v(&ev_raw1);
    EXPECT_FALSE(v.get_raw_value(&ev_raw2));
  }

  ev_raw1.type = EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;

  {
    supla_channel_and_timer_state_extended_value v(&ev_raw1);
    EXPECT_TRUE(v.get_raw_value(&ev_raw2));
  }

  EXPECT_EQ(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)), 0);
}

TEST_F(ChannelAndTimerStateExtendedValueTest, randomData) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TChannelAndTimerState_ExtendedValue);
  ev_raw1.type = EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  ((TChannelAndTimerState_ExtendedValue *)ev_raw1.value)->Timer.SenderNameSize =
      sizeof(((TChannelAndTimerState_ExtendedValue *)ev_raw1.value)
                 ->Timer.SenderName);
  ((TChannelAndTimerState_ExtendedValue *)ev_raw1.value)->Timer.SenderID = 0;
  ((TChannelAndTimerState_ExtendedValue *)ev_raw1.value)
      ->Timer.RemainingTimeMs = 0;

  supla_channel_and_timer_state_extended_value v(&ev_raw1, nullptr);
  EXPECT_EQ(v.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                   SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                   sizeof(TChannelAndTimerState_ExtendedValue));
  EXPECT_EQ(v.get_value_size(), sizeof(TChannelAndTimerState_ExtendedValue));

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_TRUE(v.get_raw_value(&ev_raw2));
  EXPECT_EQ(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)), 0);

  TChannelAndTimerState_ExtendedValue state_raw = {};
  EXPECT_TRUE(v.get_raw_value(&state_raw));

  EXPECT_EQ(memcmp(&state_raw, ev_raw2.value, v.get_value_size()), 0);
}

TEST_F(ChannelAndTimerStateExtendedValueTest, countdownEndsAt) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TChannelAndTimerState_ExtendedValue);
  ev_raw1.type = EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;

  ((TChannelAndTimerState_ExtendedValue *)ev_raw1.value)
      ->Timer.RemainingTimeMs = 5000;

  supla_channel_and_timer_state_extended_value v(&ev_raw1, nullptr);

  TChannelAndTimerState_ExtendedValue state_raw = {};
  EXPECT_TRUE(v.get_raw_value(&state_raw));

  struct timeval now;
  gettimeofday(&now, nullptr);

  EXPECT_GE(state_raw.Timer.CountdownEndsAt, now.tv_sec + 5);
  EXPECT_LE(state_raw.Timer.CountdownEndsAt, now.tv_sec + 6);
}

TEST_F(ChannelAndTimerStateExtendedValueTest, copy) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  TSuplaChannelExtendedValue ev_raw2 = {};
  TSuplaChannelExtendedValue ev_raw3 = {};

  ev_raw1.size = sizeof(TChannelAndTimerState_ExtendedValue);
  ev_raw1.type = EV_TYPE_CHANNEL_AND_TIMER_STATE_V1;

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  supla_channel_and_timer_state_extended_value v(&ev_raw1, nullptr);
  EXPECT_TRUE(v.get_raw_value(&ev_raw2));

  supla_channel_and_timer_state_extended_value *copy =
      dynamic_cast<supla_channel_and_timer_state_extended_value *>(v.copy());
  ASSERT_TRUE(copy != nullptr);

  EXPECT_TRUE(copy->get_raw_value(&ev_raw3));
  EXPECT_EQ(memcmp(&ev_raw2, &ev_raw3, sizeof(TSuplaChannelExtendedValue)), 0);

  delete copy;
}

}  // namespace testing
