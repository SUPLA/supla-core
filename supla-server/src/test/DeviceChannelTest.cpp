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

#include "devicechannel.h"  // NOLINT
#include "gtest/gtest.h"

namespace {

class DeviceChannelTest : public ::testing::Test {
 protected:
};

TEST_F(DeviceChannelTest, TemperatureHumidityClass) {
  supla_channel_temphum_value *TempHum =
      new supla_channel_temphum_value(true, 36.6, 88);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_EQ(36.6, TempHum->get_temperature());
  ASSERT_EQ(88, TempHum->get_humidity());
  delete TempHum;

  TempHum = new supla_channel_temphum_value(true, -300, -10);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_EQ(-273, TempHum->get_temperature());
  ASSERT_EQ(-1, TempHum->get_humidity());
  delete TempHum;

  TempHum = new supla_channel_temphum_value(true, 1001, 101);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_EQ(-273, TempHum->get_temperature());
  ASSERT_EQ(-1, TempHum->get_humidity());
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

  TempHum = new supla_channel_temphum_value(true, value1);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_EQ(36.6, TempHum->get_temperature());
  ASSERT_EQ(88.8, TempHum->get_humidity());

  TempHum->get_native_value(value2);
  ASSERT_EQ(0, memcmp(value1, value2, SUPLA_CHANNELVALUE_SIZE));

  delete TempHum;

  memset(value1, 0, SUPLA_CHANNELVALUE_SIZE);
  memset(value2, 0, SUPLA_CHANNELVALUE_SIZE);

  double temp = 36.6;

  memcpy(value1, &temp, sizeof(double));

  TempHum = new supla_channel_temphum_value(false, value1);
  ASSERT_FALSE(TempHum == NULL);
  ASSERT_EQ(36.6, TempHum->get_temperature());
  ASSERT_EQ(-1, TempHum->get_humidity());

  TempHum->get_native_value(value2);
  ASSERT_EQ(0, memcmp(value1, value2, SUPLA_CHANNELVALUE_SIZE));

  delete TempHum;
}
}  // namespace
