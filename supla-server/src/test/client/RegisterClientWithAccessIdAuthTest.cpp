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

#include "client/RegisterClientWithAccessIdAuthTest.h"

namespace testing {

RegisterClientWithAccessIdAuthTest::RegisterClientWithAccessIdAuthTest()
    : RegisterClientTest() {}

RegisterClientWithAccessIdAuthTest::~RegisterClientWithAccessIdAuthTest() {}

TEST_F(RegisterClientWithAccessIdAuthTest, invalidGUID) {
  TCS_SuplaRegisterClient_B register_client_b = {};

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c)
      .Times(1)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 234, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientWithAccessIdAuthTest, authFailed) {
  TCS_SuplaRegisterClient_B register_client_b = {};

  register_client_b.GUID[0] = 1;
  register_client_b.AccessID = 123;
  snprintf(register_client_b.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, "%s",
           "abcd");

  EXPECT_CALL(dba, connect).Times(2).WillRepeatedly(Return(true));
  EXPECT_CALL(dba, disconnect).Times(2);
  EXPECT_CALL(dba, start_transaction).Times(0);

  EXPECT_CALL(client_dao, access_id_auth(123, StrEq("abcd"), NotNull(),
                                         NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int access_id, char *access_id_pwd, int *user_id,
                   bool *is_enabled, bool *is_active) {
        *user_id = 15;
        *is_enabled = true;
        *is_active = true;
        return false;
      });

  EXPECT_CALL(client_dao, access_id_auth(345, StrEq("abcd"), NotNull(),
                                         NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int access_id, char *access_id_pwd, int *user_id,
                   bool *is_enabled, bool *is_active) {
        *user_id = 0;
        *is_enabled = true;
        *is_active = true;
        return true;
      });

  EXPECT_CALL(srpcAdapter, sc_async_registerclient_result_c(_))
      .Times(2)
      .WillOnce([](TSC_SuplaRegisterClientResult_C *result) {
        EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, result->result_code);
        EXPECT_EQ(20, result->activity_timeout);
        EXPECT_EQ(SUPLA_PROTO_VERSION, result->version);
        EXPECT_EQ(SUPLA_PROTO_VERSION_MIN, result->version_min);
        return 0;
      });

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 456, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());

  register_client_b.AccessID = 345;

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 55, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

TEST_F(RegisterClientWithAccessIdAuthTest, authSuccessAndRegistrationDisabled) {
  TCS_SuplaRegisterClient_B register_client_b = {};

  register_client_b.GUID[0] = 1;
  register_client_b.AccessID = 123;
  snprintf(register_client_b.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, "%s",
           "abcd");

  EXPECT_CALL(dba, connect).Times(1).WillRepeatedly(Return(true));

  EXPECT_CALL(client_dao, access_id_auth(123, StrEq("abcd"), NotNull(),
                                         NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int access_id, char *access_id_pwd, int *user_id,
                   bool *is_enabled, bool *is_active) {
        *user_id = 15;
        *is_enabled = true;
        return true;
      });

  EXPECT_CALL(dba, start_transaction).Times(1);

  EXPECT_CALL(client_dao, get_client_reg_enabled(15))
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

  rc.register_client(&register_client_b, nullptr, &srpcAdapter, &dba, nullptr,
                     &client_dao, 55, 4567, 20);

  EXPECT_GE(usecFromSetUp(), rc.get_hold_time_on_failure_usec());
}

} /* namespace testing */
