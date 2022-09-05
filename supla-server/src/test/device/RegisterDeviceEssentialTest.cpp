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

#include "device/RegisterDeviceEssentialTest.h"

namespace testing {

RegisterDeviceEssentialTest::RegisterDeviceEssentialTest()
    : RegisterDeviceTest() {}

RegisterDeviceEssentialTest::~RegisterDeviceEssentialTest() {}

void RegisterDeviceEssentialTest::SetUp() {
  RegisterDeviceTest::SetUp();

  ON_CALL(rd, get_user_id_by_email(_)).WillByDefault(Return(55));

  ON_CALL(rd, get_object_id(55, _, _))
      .WillByDefault(
          [](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
            *id = 0;
            return true;
          });

  ON_CALL(dba, connect).WillByDefault(Return(true));

  ON_CALL(dao, get_device_reg_enabled).WillByDefault(Return(true));
}

TEST_F(RegisterDeviceEssentialTest, deviceLimitExceded) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "bill@microsoft.com");

  EXPECT_CALL(dao, get_device_limit_left(55)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 169, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, noLocationAvailable) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "bill@microsoft.com");

  EXPECT_CALL(dao, get_device_limit_left(55)).Times(1).WillOnce(Return(1));

  EXPECT_CALL(dao, get_location_id(55, true)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dao, get_location_id(55, false)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 169, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, failedToAddDevice) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "bill@microsoft.com");

  EXPECT_CALL(dao, get_device_limit_left(55)).Times(1).WillOnce(Return(1));

  EXPECT_CALL(dao, get_location_id(55, true)).Times(1).WillOnce(Return(123));

  EXPECT_CALL(dao, add_device(_)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

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
                                   &dba, &dao, 169, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, deviceExistsAndIsDisabled) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "elon@spacex.com");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("elon@spacex.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rd, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 55;
        return true;
      });

  EXPECT_CALL(rd, get_authkey_hash(55, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = true;
            return true;
          });

  EXPECT_CALL(dao, get_device_id(_, _)).Times(1).WillOnce(Return(55));

  EXPECT_CALL(
      dao, get_device_variables(55, NotNull(), NotNull(), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int device_id, bool *device_enabled,
                   int *original_location_id, int *location_id,
                   bool *location_enabled) {
        *device_enabled = false;
        return 33;
      });

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_DEVICE_DISABLED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 169, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, deviceExistsAndLocationIsDisabled) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "elon@spacex.com");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("elon@spacex.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rd, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 55;
        return true;
      });

  EXPECT_CALL(rd, get_authkey_hash(55, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = true;
            return true;
          });

  EXPECT_CALL(dao, get_device_id(_, _)).Times(1).WillOnce(Return(55));

  EXPECT_CALL(
      dao, get_device_variables(55, NotNull(), NotNull(), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int device_id, bool *device_enabled,
                   int *original_location_id, int *location_id,
                   bool *location_enabled) {
        *location_enabled = false;
        return 33;
      });

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_LOCATION_DISABLED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 169, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, deviceHasLostItsLocation) {
  // Theoretically, this should not happen
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "elon@spacex.com");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("elon@spacex.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rd, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 55;
        return true;
      });

  EXPECT_CALL(rd, get_authkey_hash(55, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = true;
            return true;
          });

  EXPECT_CALL(dao, get_device_id(_, _)).Times(1).WillOnce(Return(55));

  EXPECT_CALL(
      dao, get_device_variables(55, NotNull(), NotNull(), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int device_id, bool *device_enabled,
                   int *original_location_id, int *location_id,
                   bool *location_enabled) {
        *location_id = 0;
        *location_enabled = true;
        return 33;
      });

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_LOCATION_CONFLICT, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  char result = rd.register_device(nullptr, &register_device_e, &srpcAdapter,
                                   &dba, &dao, 169, 4567, 20);

  EXPECT_EQ(result, 0);
  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

} /* namespace testing */
