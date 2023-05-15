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

#include "TimerStateExtendedValueTest.h"

#include "device/extended_value/timer_state_extended_value.h"
#include "doubles/device/extended_value/TimerStateExtendedValueMock.h"
#include "user/user.h"

namespace testing {

TEST_F(TimerStateExtendedValueTest, nulls) {
  supla_timer_state_extended_value ev(nullptr, nullptr);

  TSuplaChannelExtendedValue ev_raw = {};
  EXPECT_FALSE(ev.get_raw_value(&ev_raw));

  TTimerState_ExtendedValue ts = {};
  EXPECT_FALSE(ev.get_raw_value(&ts));
}

TEST_F(TimerStateExtendedValueTest, empty) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  supla_timer_state_extended_value ev(&ev_raw1, nullptr);

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_FALSE(ev.get_raw_value(&ev_raw2));

  TTimerState_ExtendedValue ts = {};
  EXPECT_FALSE(ev.get_raw_value(&ts));
}

TEST_F(TimerStateExtendedValueTest, withoutSenderName) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE;
  ev_raw1.type = EV_TYPE_TIMER_STATE_V1;

  supla_timer_state_extended_value ev(&ev_raw1, nullptr);

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_TRUE(ev.get_raw_value(&ev_raw2));
  EXPECT_EQ(ev.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                    SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                    sizeof(TTimerState_ExtendedValue) -
                                    SUPLA_SENDER_NAME_MAXSIZE);
  EXPECT_EQ(ev.get_value_size(),
            sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE);

  EXPECT_EQ(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)), 0);

  TTimerState_ExtendedValue ts1 = {};
  TTimerState_ExtendedValue ts2 = {};

  EXPECT_TRUE(ev.get_raw_value(&ts1));

  EXPECT_EQ(memcmp(&ts1, &ts2, sizeof(TTimerState_ExtendedValue)), 0);
}

TEST_F(TimerStateExtendedValueTest, withSenderNameAndWithoutSenderId) {
  char sender_name[] = "abcdEfg";

  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE +
                 sizeof(sender_name);
  ev_raw1.type = EV_TYPE_TIMER_STATE_V1;
  ((TTimerState_ExtendedValue *)ev_raw1.value)->SenderNameSize =
      sizeof(sender_name);
  memcpy(((TTimerState_ExtendedValue *)ev_raw1.value)->SenderName, sender_name,
         sizeof(sender_name));

  supla_timer_state_extended_value ev(&ev_raw1, nullptr);

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_TRUE(ev.get_raw_value(&ev_raw2));
  EXPECT_EQ(ev_raw2.type, EV_TYPE_TIMER_STATE_V1);
  EXPECT_EQ(ev.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                    SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                    sizeof(TTimerState_ExtendedValue) -
                                    SUPLA_SENDER_NAME_MAXSIZE +
                                    sizeof(sender_name));
  EXPECT_EQ(ev.get_value_size(), sizeof(TTimerState_ExtendedValue) -
                                     SUPLA_SENDER_NAME_MAXSIZE +
                                     sizeof(sender_name));

  TTimerState_ExtendedValue ts = {};
  EXPECT_TRUE(ev.get_raw_value(&ts));
  EXPECT_EQ(memcmp(&ts, ev_raw2.value, sizeof(TTimerState_ExtendedValue)), 0);

  EXPECT_STREQ(ts.SenderName, sender_name);
}

TEST_F(TimerStateExtendedValueTest, withSenderNameAndSenderId) {
  supla_user *user = supla_user::find(2, true);
  ASSERT_TRUE(user != nullptr);

  char sender_name[] = "abcdEfg";

  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE +
                 sizeof(sender_name);
  ev_raw1.type = EV_TYPE_TIMER_STATE_V1;
  ((TTimerState_ExtendedValue *)ev_raw1.value)->SenderNameSize =
      sizeof(sender_name);
  ((TTimerState_ExtendedValue *)ev_raw1.value)->SenderID = 12345;
  memcpy(((TTimerState_ExtendedValue *)ev_raw1.value)->SenderName, sender_name,
         sizeof(sender_name));

  TimerStateExtendedValueMock ev(nullptr, user);

  char new_name[] = "NaMe";

  EXPECT_CALL(ev, get_sender_name(Eq(12345))).WillOnce(Return(new_name));
  EXPECT_CALL(ev, get_sender_name(Eq(8910))).WillOnce(Return(""));

  ev.set_raw_value(&ev_raw1);

  TTimerState_ExtendedValue ts = {};
  EXPECT_TRUE(ev.get_raw_value(&ts));
  EXPECT_EQ(ts.SenderNameSize, sizeof(new_name));
  EXPECT_STREQ(ts.SenderName, "NaMe");

  EXPECT_EQ(ev.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                    SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                    sizeof(TTimerState_ExtendedValue) -
                                    SUPLA_SENDER_NAME_MAXSIZE +
                                    sizeof(new_name));
  EXPECT_EQ(ev.get_value_size(), sizeof(TTimerState_ExtendedValue) -
                                     SUPLA_SENDER_NAME_MAXSIZE +
                                     sizeof(new_name));

  ((TTimerState_ExtendedValue *)ev_raw1.value)->SenderID = 8910;
  ev.set_raw_value(&ev_raw1);
  EXPECT_TRUE(ev.get_raw_value(&ts));
  EXPECT_EQ(ts.SenderNameSize, 0);
  EXPECT_STREQ(ts.SenderName, "");

  EXPECT_EQ(ev.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                    SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                    sizeof(TTimerState_ExtendedValue) -
                                    SUPLA_SENDER_NAME_MAXSIZE);
  EXPECT_EQ(ev.get_value_size(),
            sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE);

  delete user;
}

TEST_F(TimerStateExtendedValueTest, countdownEndsAt) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.size = sizeof(TTimerState_ExtendedValue) - SUPLA_SENDER_NAME_MAXSIZE;
  ev_raw1.type = EV_TYPE_TIMER_STATE_V1;
  ((TTimerState_ExtendedValue *)ev_raw1.value)->RemainingTimeMs = 2000;

  supla_timer_state_extended_value ev(&ev_raw1, nullptr);

  TTimerState_ExtendedValue ts = {};
  EXPECT_TRUE(ev.get_raw_value(&ts));

  struct timeval now;
  gettimeofday(&now, nullptr);

  EXPECT_GE(ts.CountdownEndsAt, now.tv_sec + 2);
  EXPECT_LE(ts.CountdownEndsAt, now.tv_sec + 3);
}

}  // namespace testing
