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

#include <device/value/ChannelOnOffValueTest.h>

#include "device/value/channel_onoff_value.h"
#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelOnOffValueTest, voidConstructor) {
  supla_channel_onoff_value v;
  EXPECT_FALSE(v.is_on());
  EXPECT_FALSE(v.is_overcurrent_relay_off());

  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  v.get_raw_value(raw_value1);
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelOnOffValueTest, rawDataConstructor) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_onoff_value v1(raw_value);
  EXPECT_FALSE(v1.is_on());
  EXPECT_FALSE(v1.is_overcurrent_relay_off());

  ((TRelayChannel_Value*)raw_value)->hi = 1;
  supla_channel_onoff_value v2(raw_value);
  EXPECT_TRUE(v2.is_on());
  EXPECT_FALSE(v2.is_overcurrent_relay_off());

  ((TRelayChannel_Value*)raw_value)->flags =
      SUPLA_RELAY_FLAG_OVERCURRENT_RELAY_OFF;
  ((TRelayChannel_Value*)raw_value)->hi = 0;
  supla_channel_onoff_value v3(raw_value);
  EXPECT_FALSE(v3.is_on());
  EXPECT_TRUE(v3.is_overcurrent_relay_off());
}

TEST_F(ChannelOnOffValueTest, setterAndGetter) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_onoff_value v;
  v.get_raw_value(raw_value);
  EXPECT_EQ(raw_value[0], 0);
  EXPECT_FALSE(v.is_on());
  EXPECT_FALSE(v.is_overcurrent_relay_off());

  v.set_on(true);
  EXPECT_TRUE(v.is_on());
  EXPECT_FALSE(v.is_overcurrent_relay_off());
  v.get_raw_value(raw_value);
  EXPECT_EQ(raw_value[0], 1);

  v.set_overcurrent_relay_off(true);
  EXPECT_TRUE(v.is_overcurrent_relay_off());
  v.set_overcurrent_relay_off(false);
  EXPECT_FALSE(v.is_overcurrent_relay_off());
}

TEST_F(ChannelOnOffValueTest, applyChannelProperties) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  memset(raw_value1, 1, sizeof(raw_value1));

  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  supla_channel_onoff_value v(raw_value1);
  v.apply_channel_properties(0, 15, 0, 0, 0, 0, nullptr, nullptr);
  v.get_raw_value(raw_value2);

  EXPECT_EQ(memcmp(raw_value1, raw_value2, sizeof(raw_value1)), 0);

  v.apply_channel_properties(0, 14, 0, 0, 0, 0, nullptr, nullptr);
  v.get_raw_value(raw_value2);

  for (size_t a = 0; a < sizeof(raw_value2); a++) {
    ASSERT_EQ(raw_value2[a], a == 0 ? 1 : 0);
  }
}

}  // namespace testing
