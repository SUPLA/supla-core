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

#include "ChannelValueTest.h"

#include "device/value/channel_value.h"

namespace testing {

TEST_F(ChannelValueTest, voidConstructor) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  supla_channel_value v;
  v.get_raw_value(raw_value1);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelValueTest, nonVoidConstructor) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  for (unsigned char a = 0; a < SUPLA_CHANNELVALUE_SIZE; a++) {
    raw_value1[a] = a;
  }

  supla_channel_value v(raw_value1);

  v.get_raw_value(raw_value2);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
}

TEST_F(ChannelValueTest, setterAndGetter) {
  supla_channel_value v;

  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  for (unsigned char a = 0; a < SUPLA_CHANNELVALUE_SIZE; a++) {
    raw_value1[a] = a;
  }

  v.set_raw_value(raw_value1);
  raw_value1[0] = 5;

  v.get_raw_value(raw_value2);

  for (unsigned char a = 0; a < SUPLA_CHANNELVALUE_SIZE; a++) {
    EXPECT_EQ(raw_value2[a], a);
  }
}

TEST_F(ChannelValueTest, diff) {
  supla_channel_value v1;
  supla_channel_value v2;

  bool significant_change = false;

  EXPECT_TRUE(v1.is_differ(nullptr, &significant_change));
  EXPECT_TRUE(significant_change);
  EXPECT_FALSE(v1.is_differ(&v2, &significant_change));
  EXPECT_FALSE(significant_change);

  char raw[SUPLA_CHANNELVALUE_SIZE] = {};
  v1.set_raw_value(raw);

  EXPECT_FALSE(v1.is_differ(&v2, &significant_change));
  EXPECT_FALSE(significant_change);

  supla_abstract_channel_value *v3 = v1.copy();
  EXPECT_FALSE(v1.is_differ(v3, &significant_change));
  EXPECT_FALSE(significant_change);
  delete v3;

  raw[4] = 1;
  v1.set_raw_value(raw);

  EXPECT_TRUE(v1.is_differ(&v2, &significant_change));
  EXPECT_TRUE(significant_change);
}

}  // namespace testing
