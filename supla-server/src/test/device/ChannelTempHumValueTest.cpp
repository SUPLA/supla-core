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

#include <device/ChannelTmpHumValueTest.h>

#include "devicechannel.h"  // NOLINT

namespace testing {

TEST_F(ChannelTempHumValueTest, temperatureAndHumidity) {
  supla_channel_temphum_value tempHum(true, 36.6, 88);

  EXPECT_EQ(36.6, tempHum.get_temperature());
  EXPECT_EQ(88, tempHum.get_humidity());
  EXPECT_TRUE(tempHum.is_humidity_available());
}

TEST_F(ChannelTempHumValueTest, withoutHumidity) {
  supla_channel_temphum_value tempHum(false, 36.6, 88);

  EXPECT_EQ(36.6, tempHum.get_temperature());
  EXPECT_EQ(-1, tempHum.get_humidity());
  EXPECT_FALSE(tempHum.is_humidity_available());
}

TEST_F(ChannelTempHumValueTest, outOfRange) {
  supla_channel_temphum_value tempHum1(true, -300, -10);

  EXPECT_EQ(-273, tempHum1.get_temperature());
  EXPECT_EQ(-1, tempHum1.get_humidity());
  EXPECT_TRUE(tempHum1.is_humidity_available());

  supla_channel_temphum_value tempHum2(true, 1001, 101);

  EXPECT_EQ(-273, tempHum2.get_temperature());
  EXPECT_EQ(-1, tempHum2.get_humidity());
  EXPECT_TRUE(tempHum2.is_humidity_available());
}

TEST_F(ChannelTempHumValueTest, nativeValue) {
  char value1[SUPLA_CHANNELVALUE_SIZE];
  char value2[SUPLA_CHANNELVALUE_SIZE];

  memset(value1, 0, SUPLA_CHANNELVALUE_SIZE);
  memset(value2, 0, SUPLA_CHANNELVALUE_SIZE);

  int n;

  n = 36600;
  memcpy(value1, &n, 4);

  n = 88800;
  memcpy(&value1[4], &n, 4);

  supla_channel_temphum_value tempHum1(true, value1);

  EXPECT_EQ(36.6, tempHum1.get_temperature());
  EXPECT_EQ(88.8, tempHum1.get_humidity());
  EXPECT_TRUE(tempHum1.is_humidity_available());

  tempHum1.get_raw_value(value2);
  ASSERT_EQ(0, memcmp(value1, value2, SUPLA_CHANNELVALUE_SIZE));

  memset(value1, 0, SUPLA_CHANNELVALUE_SIZE);
  memset(value2, 0, SUPLA_CHANNELVALUE_SIZE);

  double temp = 36.6;

  memcpy(value1, &temp, sizeof(double));

  supla_channel_temphum_value tempHum2(false, value1);

  EXPECT_EQ(36.6, tempHum2.get_temperature());
  EXPECT_EQ(-1, tempHum2.get_humidity());
  EXPECT_FALSE(tempHum2.is_humidity_available());

  tempHum2.get_raw_value(value2);
  EXPECT_EQ(0, memcmp(value1, value2, SUPLA_CHANNELVALUE_SIZE));
}
}  // namespace testing
