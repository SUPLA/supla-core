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
  gettimeofday(&setUpTime, nullptr);
}

void RegisterDeviceTest::TearDown() { Test::TearDown(); }

unsigned int RegisterDeviceTest::msecFromSetUp(void) {
  struct timeval now = {};
  gettimeofday(&now, nullptr);

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

  char result = rd.register_device(&register_device_c, nullptr, &srpcAdapter,
                                   &dba, &dao, 55, 4567, 20);

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

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);
}

TEST_F(RegisterDeviceTest, invalidAuthkey) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_AUTHKEY_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);
}

TEST_F(RegisterDeviceTest, dbConnectionFailed) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(false));

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE,
                  result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);
}

TEST_F(RegisterDeviceTest, locationAuthFailed) {
  TDS_SuplaRegisterDevice_C register_device_c = {};

  register_device_c.GUID[0] = 1;
  register_device_c.LocationID = 123;
  snprintf(register_device_c.LocationPWD, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "abcd");

  EXPECT_CALL(dba, connect).Times(2).WillRepeatedly(Return(true));

  EXPECT_CALL(dao, location_auth(123, StrEq("abcd"), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int location_id, char *location_pwd, int *user_id,
                   bool *is_enabled) {
        *user_id = 15;
        *is_enabled = true;
        return false;
      });

  EXPECT_CALL(dao, location_auth(345, StrEq("abcd"), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int location_id, char *location_pwd, int *user_id,
                   bool *is_enabled) {
        *user_id = 0;
        *is_enabled = true;
        return true;
      });

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(2)
      .WillRepeatedly([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(&register_device_c, nullptr, &srpcAdapter,
                                   &dba, &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);

  register_device_c.LocationID = 345;

  result = rd.register_device(&register_device_c, nullptr, &srpcAdapter, &dba,
                              &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);
}

TEST_F(RegisterDeviceTest, locationAuthSuccessButRegistrtionIsDisabled) {
  TDS_SuplaRegisterDevice_C register_device_c = {};

  register_device_c.GUID[0] = 1;
  register_device_c.LocationID = 123;
  snprintf(register_device_c.LocationPWD, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "abcd");

  EXPECT_CALL(dba, connect).Times(1).WillRepeatedly(Return(true));

  EXPECT_CALL(dao, location_auth(123, StrEq("abcd"), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int location_id, char *location_pwd, int *user_id,
                   bool *is_enabled) {
        *user_id = 15;
        *is_enabled = true;
        return true;
      });

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dao, get_device_reg_enabled(15)).Times(1).WillOnce(Return(false));

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillRepeatedly([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_REGISTRATION_DISABLED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(&register_device_c, nullptr, &srpcAdapter,
                                   &dba, &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GT(msecFromSetUp(), 2000);
}

} /* namespace testing */
