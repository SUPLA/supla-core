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

#include "client/RegisterClientWithEmailAuthTest.h"

namespace testing {

RegisterClientWithEmailAuthTest::RegisterClientWithEmailAuthTest()
    : RegisterClientTest() {}

RegisterClientWithEmailAuthTest::~RegisterClientWithEmailAuthTest() {}

TEST_F(RegisterClientWithEmailAuthTest, invalidGUID) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientWithEmailAuthTest, invalidAuthkey) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_AUTHKEY_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientWithEmailAuthTest, dbaConnectionFailed) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(false));
  EXPECT_CALL(dba, disconnect).Times(0);
  EXPECT_CALL(rc, revoke_superuser_authorization).Times(1);

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

TEST_F(RegisterClientWithEmailAuthTest, emailNotFound) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "maria@sklodowska-curie.p!");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("maria@sklodowska-curie.p!")))
      .Times(1)
      .WillOnce(Return(0));

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

TEST_F(RegisterClientWithEmailAuthTest, getObjectIdWithFail) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "maria@sklodowska-curie.pl");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("maria@sklodowska-curie.pl")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _)).Times(1).WillOnce(Return(false));

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

TEST_F(RegisterClientWithEmailAuthTest,
       clientNotExistsAndRegistrationDisabled) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "maria@sklodowska-curie.pl");

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("maria@sklodowska-curie.pl")))
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

  EXPECT_CALL(dba, rollback).Times(1);

  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_REGISTRATION_DISABLED, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(nullptr, &register_client_d, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientWithEmailAuthTest, getAuthKeyWithFail) {
  TCS_SuplaRegisterClient_D register_client_d = {};

  register_client_d.GUID[0] = 1;
  register_client_d.AuthKey[0] = 2;

  snprintf(register_client_d.Email, SUPLA_EMAIL_MAXSIZE, "%s",
           "maria@sklodowska-curie.pl");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, disconnect).Times(1);

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("maria@sklodowska-curie.pl")))
      .Times(1)
      .WillOnce(Return(25));

  EXPECT_CALL(rc, get_object_id(25, _, _))
      .Times(1)
      .WillOnce([](int user_id, const char guid[SUPLA_GUID_SIZE], int *id) {
        *id = 55;
        return true;
      });

  EXPECT_CALL(rc, get_authkey_hash(55, NotNull(), NotNull()))
      .Times(1)
      .WillOnce(
          [](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
            *is_null = false;
            return false;
          });

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
