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

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, is_connected).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, disconnect).Times(1);

  ON_CALL(dao, get_device_reg_enabled).WillByDefault(Return(true));

  ON_CALL(dao, get_device_limit_left).WillByDefault(Return(100));
}

TEST_F(RegisterDeviceEssentialTest, deviceLimitExceded) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "bill@microsoft.com");

  EXPECT_CALL(dao, get_device_limit_left(55)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, noLocationAvailable) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "bill@microsoft.com");

  EXPECT_CALL(dao, get_device_limit_left(55)).Times(1).WillOnce(Return(1));

  EXPECT_CALL(dao, get_location_id(55, true)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dao, get_location_id(55, false)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, failedToAddDevice) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "bill@microsoft.com");

  EXPECT_CALL(dao, get_device_limit_left(55)).Times(1).WillOnce(Return(1));

  EXPECT_CALL(dao, get_location_id(55, true)).Times(1).WillOnce(Return(123));

  EXPECT_CALL(dao, add_device(_)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dba, rollback).Times(1);

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

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, gettingDeviceVariablesFailed) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "elon@spacex.com");

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
                   bool *location_enabled) { return false; });

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

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, deviceExistsAndIsDisabled) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "elon@spacex.com");

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
        return true;
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

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, deviceExistsAndLocationIsDisabled) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "elon@spacex.com");

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
        return true;
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

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, deviceHasLostItsLocation) {
  // Theoretically, this should not happen
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "elon@spacex.com");

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
        return true;
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

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, locationConflict) {
  TDS_SuplaRegisterDevice_C register_device_c = {};

  register_device_c.GUID[0] = 1;
  register_device_c.LocationID = 123;
  snprintf(register_device_c.LocationPWD, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
           "abcd");

  EXPECT_CALL(dao, location_auth(123, StrEq("abcd"), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int location_id, char *location_pwd, int *user_id,
                   bool *is_enabled) {
        *user_id = 15;
        *is_enabled = true;
        return true;
      });

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(dao, get_device_id(_, _)).Times(1).WillOnce(Return(55));

  EXPECT_CALL(
      dao, get_device_variables(_, NotNull(), NotNull(), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int device_id, bool *device_enabled,
                   int *original_location_id, int *location_id,
                   bool *location_enabled) {
        *location_id = 45;
        *original_location_id = 46;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillRepeatedly([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_LOCATION_CONFLICT, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(&register_device_c, nullptr, &srpcAdapter, &dba, &dao, 456,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, wrongNumberOfChannels) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "elon@spacex.com");

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
        *location_id = 155;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dao, get_device_channel_count(55)).Times(1).WillOnce(Return(1));

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_CHANNEL_CONFLICT, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, channelTypeChanged) {
  TDS_SuplaRegisterDevice_E register_device_e = {};
  register_device_e.channel_count = 2;
  register_device_e.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
  register_device_e.channels[1].Number = 1;
  register_device_e.channels[1].Type = SUPLA_CHANNELTYPE_DIMMER;

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "elon@spacex.com");

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
        *location_id = 155;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dao, get_device_channel_count(55)).Times(0);

  EXPECT_CALL(dao, get_channel_id_and_type(55, 0, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = SUPLA_CHANNELTYPE_RELAY;
        return 1;
      });

  EXPECT_CALL(dao, get_channel_id_and_type(55, 1, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = SUPLA_CHANNELTYPE_RELAY;
        return 2;
      });

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_CHANNEL_CONFLICT, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, cantUdateDevice) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "cheops@giza.com");

  snprintf(register_device_e.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "22.09");

  snprintf(register_device_e.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
           "Torch Switch");

  EXPECT_CALL(dba, start_transaction).Times(1);
  EXPECT_CALL(dba, rollback).Times(1);
  EXPECT_CALL(dba, commit).Times(0);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("cheops@giza.com")))
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
        *location_id = 155;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dao, get_device_channel_count(55)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dao, on_new_device(_)).Times(0);
  EXPECT_CALL(dao, update_device(55, 0, NotNull(), StrEq("Torch Switch"), 4567,
                                 StrEq("22.09"), SUPLA_PROTO_VERSION, 0))
      .Times(1)
      .WillOnce(Return(false));

  EXPECT_CALL(rd, on_registration_success).Times(0);

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

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, cfgModeRequested) {
  TDS_SuplaRegisterDevice_E register_device_e = {};

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;
  register_device_e.Flags = SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "cheops@giza.com");

  snprintf(register_device_e.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "22.09");

  snprintf(register_device_e.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
           "Torch Switch");

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("cheops@giza.com")))
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
        *location_id = 155;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dao, update_device).Times(1).WillOnce(Return(true));

  EXPECT_CALL(rd, is_prev_entering_cfg_mode).Times(1).WillOnce(Return(true));
  EXPECT_CALL(rd, on_registration_success).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_CFG_MODE_REQUESTED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_EQ(55, rd.get_device_id());
  EXPECT_FALSE(rd.is_channel_added());
  EXPECT_LT(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest,
       successFullRegistrationWithExistingDeviceAndChannels) {
  TDS_SuplaRegisterDevice_E register_device_e = {};
  register_device_e.channel_count = 2;
  register_device_e.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
  register_device_e.channels[1].Number = 1;
  register_device_e.channels[1].Type = SUPLA_CHANNELTYPE_DIMMER;

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;
  register_device_e.Flags = 778899 & ~SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "cheops@giza.com");

  snprintf(register_device_e.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "22.09");

  snprintf(register_device_e.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
           "Torch Switch");

  EXPECT_CALL(dba, start_transaction).Times(1);
  EXPECT_CALL(dba, rollback).Times(0);
  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("cheops@giza.com")))
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
        *location_id = 155;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dao, get_device_channel_count(55)).Times(1).WillOnce(Return(2));

  EXPECT_CALL(dao, get_channel_id_and_type(55, 0, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = SUPLA_CHANNELTYPE_RELAY;
        return 1;
      });

  EXPECT_CALL(dao, get_channel_id_and_type(55, 1, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = SUPLA_CHANNELTYPE_DIMMER;
        return 2;
      });

  EXPECT_CALL(dao, on_new_device(_)).Times(0);
  EXPECT_CALL(dao, update_device(55, 0, NotNull(), StrEq("Torch Switch"), 4567,
                                 StrEq("22.09"), SUPLA_PROTO_VERSION, 778899))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(rd, on_registration_success).Times(1);
  EXPECT_CALL(rd, is_prev_entering_cfg_mode).Times(0);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TRUE, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_EQ(55, rd.get_device_id());
  EXPECT_FALSE(rd.is_channel_added());
  EXPECT_LT(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, addChannelsToExistingDevice) {
  TDS_SuplaRegisterDevice_E register_device_e = {};
  register_device_e.channel_count = 2;
  register_device_e.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
  register_device_e.channels[1].Number = 1;
  register_device_e.channels[1].Type = SUPLA_CHANNELTYPE_DIMMER;
  register_device_e.channels[1].Default = SUPLA_CHANNELFNC_DIMMER;

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;
  register_device_e.Flags = 778899;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "cheops@giza.com");

  snprintf(register_device_e.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "22.09");

  snprintf(register_device_e.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
           "Torch Switch");

  EXPECT_CALL(dba, start_transaction).Times(1);
  EXPECT_CALL(dba, rollback).Times(0);
  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("cheops@giza.com")))
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
        *location_id = 155;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dao, get_device_channel_count(55)).Times(1).WillOnce(Return(2));

  EXPECT_CALL(dao, get_channel_id_and_type(55, 0, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = SUPLA_CHANNELTYPE_RELAY;
        return 1;
      });

  EXPECT_CALL(dao, get_channel_id_and_type(55, 1, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = 0;
        return 0;
      });

  EXPECT_CALL(dao, add_channel(55, 1, SUPLA_CHANNELTYPE_DIMMER,
                               SUPLA_CHANNELFNC_DIMMER, 0, 0, 0, 0, 25))
      .Times(1)
      .WillOnce(Return(456));

  EXPECT_CALL(dao, on_channel_added(55, 456)).Times(1);

  EXPECT_CALL(dao, on_new_device(_)).Times(0);
  EXPECT_CALL(dao, update_device(55, 0, NotNull(), StrEq("Torch Switch"), 4567,
                                 StrEq("22.09"), SUPLA_PROTO_VERSION, 778899))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(rd, on_registration_success).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TRUE, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_EQ(55, rd.get_device_id());
  EXPECT_TRUE(rd.is_channel_added());
  EXPECT_LT(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, failedToAddChannel) {
  TDS_SuplaRegisterDevice_E register_device_e = {};
  register_device_e.channel_count = 2;
  register_device_e.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
  register_device_e.channels[1].Number = 1;
  register_device_e.channels[1].Type = SUPLA_CHANNELTYPE_DIMMER;
  register_device_e.channels[1].Default = SUPLA_CHANNELFNC_DIMMER;

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;
  register_device_e.Flags = 778899;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "cheops@giza.com");

  snprintf(register_device_e.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "22.09");

  snprintf(register_device_e.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
           "Torch Switch");

  EXPECT_CALL(dba, start_transaction).Times(1);
  EXPECT_CALL(dba, rollback).Times(1);
  EXPECT_CALL(dba, commit).Times(0);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("cheops@giza.com")))
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
        *location_id = 155;
        *location_enabled = true;
        return true;
      });

  EXPECT_CALL(dao, get_device_channel_count).Times(0);

  EXPECT_CALL(dao, get_channel_id_and_type(55, 0, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = SUPLA_CHANNELTYPE_RELAY;
        return 1;
      });

  EXPECT_CALL(dao, get_channel_id_and_type(55, 1, NotNull()))
      .Times(1)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = 0;
        return 0;
      });

  EXPECT_CALL(dao, add_channel(55, 1, SUPLA_CHANNELTYPE_DIMMER,
                               SUPLA_CHANNELFNC_DIMMER, 0, 0, 0, 0, 25))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(dao, on_channel_added).Times(0);

  EXPECT_CALL(dao, on_new_device).Times(0);
  EXPECT_CALL(dao, update_device).Times(0);

  EXPECT_CALL(rd, on_registration_success).Times(0);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_CHANNEL_CONFLICT, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_GE(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

TEST_F(RegisterDeviceEssentialTest, addDeviceAndChannels) {
  TDS_SuplaRegisterDevice_E register_device_e = {};
  register_device_e.channel_count = 2;
  register_device_e.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
  register_device_e.channels[1].Number = 1;
  register_device_e.channels[1].Type = SUPLA_CHANNELTYPE_DIMMER;
  register_device_e.channels[1].Default = SUPLA_CHANNELFNC_DIMMER;

  register_device_e.GUID[0] = 1;
  register_device_e.AuthKey[0] = 2;
  register_device_e.Flags = 778899;
  register_device_e.ManufacturerID = SUPLA_MFR_ACSOFTWARE;
  register_device_e.ProductID = 177;

  snprintf(register_device_e.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "cheops@giza.com");

  snprintf(register_device_e.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "22.09");

  snprintf(register_device_e.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
           "Torch Switch");

  EXPECT_CALL(dba, start_transaction).Times(1);
  EXPECT_CALL(dba, rollback).Times(0);
  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(rd, get_user_id_by_email(StrEq("cheops@giza.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rd, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(rd, get_authkey_hash).Times(0);

  EXPECT_CALL(dao, get_device_id(_, _)).Times(1).WillOnce(Return(0));

  EXPECT_CALL(dao, get_device_variables).Times(0);

  EXPECT_CALL(dao, get_location_id(25, true)).Times(1).WillOnce(Return(88));

  EXPECT_CALL(dao, add_device(88, NotNull(), NotNull(), StrEq("Torch Switch"),
                              4567, StrEq("22.09"), SUPLA_PROTO_VERSION,
                              SUPLA_MFR_ACSOFTWARE, 177, 778899))
      .Times(1);

  EXPECT_CALL(dao, add_device(25)).Times(1).WillOnce(Return(55));

  EXPECT_CALL(dao, get_channel_id_and_type(55, _, NotNull()))
      .Times(2)
      .WillOnce([](int device_id, int channel_number, int *type) {
        *type = 0;
        return 0;
      });

  EXPECT_CALL(dao,
              add_channel(55, 0, SUPLA_CHANNELTYPE_RELAY, 0, 0, 0, 0, 0, 25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(dao, add_channel(55, 1, SUPLA_CHANNELTYPE_DIMMER,
                               SUPLA_CHANNELFNC_DIMMER, 0, 0, 0, 0, 25))
      .Times(1)
      .WillOnce(Return(2));

  EXPECT_CALL(dao, get_device_channel_count).Times(1).WillOnce(Return(2));

  EXPECT_CALL(dao, on_channel_added).Times(2);

  EXPECT_CALL(dao, on_new_device).Times(1);
  EXPECT_CALL(dao, update_device).Times(0);

  EXPECT_CALL(rd, on_registration_success).Times(1);

  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .Times(1)
      .WillOnce([](TSD_SuplaRegisterDeviceResult *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TRUE, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rd.register_device(nullptr, &register_device_e, &srpcAdapter, &dba, &dao, 169,
                     4567, 20);

  EXPECT_EQ(55, rd.get_device_id());
  EXPECT_TRUE(rd.is_channel_added());
  EXPECT_LT(usecFromSetUp(), rd.get_hold_time_on_failure_usec());
}

} /* namespace testing */
