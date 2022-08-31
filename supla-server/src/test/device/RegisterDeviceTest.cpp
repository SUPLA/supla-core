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

#include "device/RegisterDeviceTest.h"

namespace testing {

RegisterDeviceTest::RegisterDeviceTest() : Test() {}

RegisterDeviceTest::~RegisterDeviceTest() {}

void RegisterDeviceTest::SetUp() {
  Test::SetUp();
  gettimeofday(&setUpTime, NULL);
}

void RegisterDeviceTest::TearDown() { Test::TearDown(); }

unsigned int RegisterDeviceTest::msecFromSetUp(void) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  return ((now.tv_sec * 1000000 + now.tv_usec) -
          (setUpTime.tv_sec * 1000000 + setUpTime.tv_usec)) /
         1000;
}

TEST_F(RegisterDeviceTest, invalidGUID_c) {
  TDS_SuplaRegisterDevice_C register_device_c = {};

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(&register_device_c, NULL, &srpcAdapter, &dba,
                                   &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);
}

TEST_F(RegisterDeviceTest, invalidGUID_e) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(NULL, &register_device_e, &srpcAdapter, &dba,
                                   &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);
}

} /* namespace testing */
