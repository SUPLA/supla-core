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

#include "TestHelper.h"
#include "device/extended_value/channel_state_extended_value.h"

namespace testing {

TEST_F(ChannelStateExtendedValueTest, defaultConstructor) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  TSuplaChannelExtendedValue ev_raw2 = {};

  ev_raw1.size = sizeof(TChannelState_ExtendedValue);

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  {
    supla_channel_state_extended_value v(&ev_raw1);
    EXPECT_FALSE(v.get_raw_value(&ev_raw2));
  }

  ev_raw1.type = EV_TYPE_CHANNEL_STATE_V1;

  {
    supla_channel_state_extended_value v(&ev_raw1);
    EXPECT_TRUE(v.get_raw_value(&ev_raw2));
  }

  EXPECT_EQ(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)), 0);
}

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

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  supla_channel_state_extended_value ev(&ev_raw1);

  TSuplaChannelExtendedValue ev_raw2 = {};
  EXPECT_EQ(ev.get_value_size(), sizeof(TChannelState_ExtendedValue));
  EXPECT_EQ(ev.get_real_size(), sizeof(TSuplaChannelExtendedValue) -
                                    SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                                    sizeof(TChannelState_ExtendedValue));
  EXPECT_TRUE(ev.get_raw_value(&ev_raw2));
  EXPECT_EQ(memcmp(&ev_raw1, &ev_raw2, sizeof(TSuplaChannelExtendedValue)), 0);
}

TEST_F(ChannelStateExtendedValueTest, copy) {
  TSuplaChannelExtendedValue ev_raw1 = {};
  ev_raw1.type = EV_TYPE_CHANNEL_STATE_V1;
  ev_raw1.size = sizeof(TChannelState_ExtendedValue);

  TestHelper::randomize(ev_raw1.value, ev_raw1.size);

  ((TTimerState_ExtendedValue *)ev_raw1.value)->SenderNameSize = 0;

  supla_channel_state_extended_value ev(&ev_raw1);

  supla_channel_state_extended_value *copy =
      dynamic_cast<supla_channel_state_extended_value *>(ev.copy());
  ASSERT_TRUE(copy != nullptr);

  TSuplaChannelExtendedValue ev1 = {};
  TSuplaChannelExtendedValue ev2 = {};
  ev.get_raw_value(&ev1);
  copy->get_raw_value(&ev2);

  EXPECT_EQ(memcmp(&ev1, &ev2, sizeof(TSuplaChannelExtendedValue)), 0);

  delete copy;
}

}  // namespace testing
