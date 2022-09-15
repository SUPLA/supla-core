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

#include "client/RegisterClientEssentialTest.h"

namespace testing {

RegisterClientEssentialTest::RegisterClientEssentialTest()
    : RegisterClientTest() {}

RegisterClientEssentialTest::~RegisterClientEssentialTest() {}

TEST_F(RegisterClientEssentialTest, incorrectSuperuserPassword) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");
  snprintf(register_client_d.Password, SUPLA_PASSWORD_MAXSIZE, "%s",
           "Bushnell.xyz");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(false));

  EXPECT_CALL(rc, superuser_authorize(25, StrEq("nolan@atari.com"),
                                      StrEq("Bushnell.xyz")))
      .Times(1);

  EXPECT_CALL(rc, is_superuser_authorized).Times(1).WillOnce(Return(false));

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest, clientLimitExceeded) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       getAccessIdWhenClientCountIsGreaterThanZero) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao, get_client_count(25)).Times(1).WillOnce(Return(1));

  EXPECT_CALL(rc, is_superuser_authorized).Times(1).WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_access_id(25, true, true))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(client_dao, get_access_id(25, false, true))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(client_dao, get_access_id(25, false, false))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE,
                  result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       clientCountIsZeroAndSuperuserIsNotAuthorized) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao, get_client_count(25)).Times(1).WillOnce(Return(1));

  EXPECT_CALL(rc, is_superuser_authorized).Times(1).WillOnce(Return(false));

  EXPECT_CALL(client_dao, get_access_id).Times(0);

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE,
                  result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest, failedToAddTheClient) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");
  snprintf(register_client_d.Name, SUPLA_CLIENT_NAME_MAXSIZE, "%s",
           "Atari 800XL");
  snprintf(register_client_d.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "1.0");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao,
              add_client(0, NotNull(), NotNull(), StrEq("Atari 800XL"), 4567,
                         StrEq("1.0"), SUPLA_PROTO_VERSION, 25))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE,
                  result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       successfullyAddedClientWithAnUnasignedAccessIDAndWithoutPassword) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao, add_client).Times(1).WillOnce(Return(88));

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       successfullyAddedClientWithAnUnasignedAccessIDAndPasswordProvided) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");
  snprintf(register_client_d.Password, SUPLA_PASSWORD_MAXSIZE, "%s",
           "Bushnell.xyz");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao, add_client).Times(1).WillOnce(Return(88));

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       successfullyAddedClientWithDisabledAccessId) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao, add_client).Times(1).WillOnce(Return(88));

  EXPECT_CALL(client_dao, get_client_access_id(88, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(Return(155));

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_ACCESSID_DISABLED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       successfullyAddedClientWithInActiveAccessId) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao, add_client).Times(1).WillOnce(Return(88));

  EXPECT_CALL(client_dao, get_client_access_id(88, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int client_id, bool *accessid_enabled, bool *accessid_active) {
            *accessid_enabled = true;
            return 155;
          });

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_ACCESSID_INACTIVE, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest, successfullyAddedClient) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 0;
        return true;
      });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(25))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_client_limit_left(25))
      .Times(1)
      .WillOnce(Return(1));

  EXPECT_CALL(client_dao, add_client).Times(1).WillOnce(Return(88));

  EXPECT_CALL(client_dao, get_client_access_id(88, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int client_id, bool *accessid_enabled, bool *accessid_active) {
            *accessid_enabled = true;
            *accessid_active = true;
            return 155;
          });

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(rc, on_registraction_success).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_TRUE, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_LT(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest, clientExistsAndIsDisabled) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 987;
        return true;
      });

  EXPECT_CALL(rc, get_authkey_hash(987, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = true;
            return true;
          });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_id(25, NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(client_dao, get_client_variables(987, NotNull(), NotNull(),
                                               NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int client_id, bool *client_enabled, int *access_id,
                   bool *accessid_enabled, bool *accessid_active) {
        *client_enabled = false;
        return client_id;
      });

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, commit).Times(0);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_CLIENT_DISABLED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest, clientExistsAndAccessIdIsNotSet) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");
  snprintf(register_client_d.Name, SUPLA_CLIENT_NAME_MAXSIZE, "%s",
           "Atari 800XL");
  snprintf(register_client_d.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s", "1.0");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 987;
        return true;
      });

  EXPECT_CALL(rc, get_authkey_hash(987, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = true;
            return true;
          });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_id(25, NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(client_dao, get_client_variables(987, NotNull(), NotNull(),
                                               NotNull(), NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(client_dao,
              update_client(987, 0, NotNull(), StrEq("Atari 800XL"), 4567,
                            StrEq("1.0"), SUPLA_PROTO_VERSION))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       clientExistsAndAccessIdIsNotSet_PasswordProvided_UnableToGetAccessId) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");
  snprintf(register_client_d.Password, SUPLA_PASSWORD_MAXSIZE, "%s",
           "Bushnell.xyz");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 987;
        return true;
      });

  EXPECT_CALL(rc, get_authkey_hash(987, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = true;
            return true;
          });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_id(25, NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(client_dao, get_client_variables(987, NotNull(), NotNull(),
                                               NotNull(), NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  Sequence s1;

  EXPECT_CALL(rc, is_superuser_authorized)
      .Times(1)
      .InSequence(s1)
      .WillOnce(Return(false));

  EXPECT_CALL(rc, superuser_authorize(25, StrEq("nolan@atari.com"),
                                      StrEq("Bushnell.xyz")))
      .Times(1)
      .InSequence(s1);

  EXPECT_CALL(rc, is_superuser_authorized)
      .Times(1)
      .InSequence(s1)
      .WillOnce(Return(true));

  EXPECT_CALL(client_dao, get_access_id(25, true, true))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(client_dao, update_client(987, 0, NotNull(), NotNull(), 4567,
                                        NotNull(), SUPLA_PROTO_VERSION))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientEssentialTest,
       clientExistsAndAccessIdIsNotSet_IncorrectPasswordProvided) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "nolan@atari.com");
  snprintf(register_client_d.Password, SUPLA_PASSWORD_MAXSIZE, "%s",
           "Bushnell.xyz");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("nolan@atari.com")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 987;
        return true;
      });

  EXPECT_CALL(rc, get_authkey_hash(987, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = true;
            return true;
          });

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_id(25, NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(client_dao, get_client_variables(987, NotNull(), NotNull(),
                                               NotNull(), NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(rc, superuser_authorize).Times(1);

  EXPECT_CALL(rc, is_superuser_authorized)
      .Times(2)
      .WillRepeatedly(Return(false));

  EXPECT_CALL(client_dao, get_access_id).Times(0);

  EXPECT_CALL(client_dao, update_client(987, 0, NotNull(), NotNull(), 4567,
                                        NotNull(), SUPLA_PROTO_VERSION))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(dba, rollback).Times(0);

  EXPECT_CALL(dba, commit).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

} /* namespace testing */
