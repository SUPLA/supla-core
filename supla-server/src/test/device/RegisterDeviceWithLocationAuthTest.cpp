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

#include "device/RegisterDeviceWithLocationAuthTest.h"

namespace testing {

RegisterDeviceWithLocationAuthTest::RegisterDeviceWithLocationAuthTest()
    : RegisterDeviceTest() {}

RegisterDeviceWithLocationAuthTest::~RegisterDeviceWithLocationAuthTest() {}

TEST_F(RegisterDeviceWithLocationAuthTest, invalidGUID) {
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
                                   &dba, &dao, 456, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceWithLocationAuthTest, authFailed) {
  TDS_SuplaRegisterDevice_C register_device_c = {};

  register_device_c.GUID[0] = 1;
  register_device_c.LocationID = 123;
  snprintf(register_device_c.LocationPWD, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "abcd");

  EXPECT_CALL(dba, connect).Times(2).WillRepeatedly(Return(true));
  EXPECT_CALL(dba, disconnect).Times(2);
  EXPECT_CALL(dba, start_transaction).Times(0);

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
                                   &dba, &dao, 456, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());

  register_device_c.LocationID = 345;

  result = rd.register_device(&register_device_c, nullptr, &srpcAdapter, &dba,
                              &dao, 55, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceWithLocationAuthTest, authSuccessAndRegistrationDisabled) {
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

  EXPECT_CALL(dba, disconnect).Times(1);

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
                                   &dba, &dao, 456, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

} /* namespace testing */
