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

#include "DeviceChannelTest.h"
#include "../device/devicechannel.h"  // NOLINT
#include "gtest/gtest.h"

namespace {

class DeviceChannelTest : public ::testing::Test {
 protected:
};

TEST_F(DeviceChannelTest, TemperatureHumidityClass) {
  supla_channel_temphum *TempHum =
      new supla_channel_temphum(true, 111, 36.6, 88);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_TRUE(TempHum->isTempAndHumidity());
  ASSERT_EQ(111, TempHum->getChannelId());
  ASSERT_EQ(36.6, TempHum->getTemperature());
  ASSERT_EQ(88, TempHum->getHumidity());
  delete TempHum;

  TempHum = new supla_channel_temphum(true, 200, -300, -10);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_TRUE(TempHum->isTempAndHumidity());
  ASSERT_EQ(200, TempHum->getChannelId());
  ASSERT_EQ(-273, TempHum->getTemperature());
  ASSERT_EQ(-1, TempHum->getHumidity());
  delete TempHum;

  TempHum = new supla_channel_temphum(true, 200, 1001, 101);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_TRUE(TempHum->isTempAndHumidity());
  ASSERT_EQ(200, TempHum->getChannelId());
  ASSERT_EQ(-273, TempHum->getTemperature());
  ASSERT_EQ(-1, TempHum->getHumidity());
  delete TempHum;

  char value1[SUPLA_CHANNELVALUE_SIZE];
  char value2[SUPLA_CHANNELVALUE_SIZE];

  memset(value1, 0, SUPLA_CHANNELVALUE_SIZE);
  memset(value2, 0, SUPLA_CHANNELVALUE_SIZE);

  int n;

  n = 36600;
  memcpy(value1, &n, 4);

  n = 88800;
  memcpy(&value1[4], &n, 4);

  TempHum = new supla_channel_temphum(true, 202, value1);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_TRUE(TempHum->isTempAndHumidity());
  ASSERT_EQ(202, TempHum->getChannelId());
  ASSERT_EQ(36.6, TempHum->getTemperature());
  ASSERT_EQ(88.8, TempHum->getHumidity());

  TempHum->toValue(value2);
  ASSERT_EQ(0, memcmp(value1, value2, SUPLA_CHANNELVALUE_SIZE));

  delete TempHum;

  memset(value1, 0, SUPLA_CHANNELVALUE_SIZE);
  memset(value2, 0, SUPLA_CHANNELVALUE_SIZE);

  double temp = 36.6;

  memcpy(value1, &temp, sizeof(double));

  TempHum = new supla_channel_temphum(false, 203, value1);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_FALSE(TempHum->isTempAndHumidity());
  ASSERT_EQ(203, TempHum->getChannelId());
  ASSERT_EQ(36.6, TempHum->getTemperature());

  TempHum->toValue(value2);
  ASSERT_EQ(0, memcmp(value1, value2, SUPLA_CHANNELVALUE_SIZE));

  delete TempHum;
}
}  // namespace
