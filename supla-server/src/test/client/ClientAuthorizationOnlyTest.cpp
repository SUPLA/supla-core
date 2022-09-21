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

#include "client/ClientAuthorizationOnlyTest.h"

namespace testing {

using std::shared_ptr;

ClientAuthorizationOnlyTest::ClientAuthorizationOnlyTest()
    : RegisterClientTest() {}

ClientAuthorizationOnlyTest::~ClientAuthorizationOnlyTest() {}

TEST_F(ClientAuthorizationOnlyTest, invalidGUID) {
  shared_ptr<supla_client> client;
  TCS_ClientAuthorizationDetails auth = {};

  rc.authenticate(client, &auth, &srpcAdapter, &dba, nullptr, &client_dao,
                  true);

  EXPECT_EQ(SUPLA_RESULTCODE_GUID_ERROR, rc.get_result_code());
}

TEST_F(ClientAuthorizationOnlyTest, invalidAuthkey) {
  shared_ptr<supla_client> client;
  TCS_ClientAuthorizationDetails auth = {};

  auth.GUID[0] = 1;
  snprintf(auth.Email, sizeof(auth.Email), "%s", "ted@atari.com");

  rc.authenticate(client, &auth, &srpcAdapter, &dba, nullptr, &client_dao,
                  true);

  EXPECT_EQ(SUPLA_RESULTCODE_AUTHKEY_ERROR, rc.get_result_code());
}

TEST_F(ClientAuthorizationOnlyTest, dbaConnectionFailed) {
  shared_ptr<supla_client> client;
  TCS_ClientAuthorizationDetails auth = {};

  auth.GUID[0] = 1;
  auth.AuthKey[0] = 2;

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(false));

  rc.authenticate(client, &auth, &srpcAdapter, &dba, nullptr, &client_dao,
                  true);

  EXPECT_EQ(SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE, rc.get_result_code());
}

TEST_F(ClientAuthorizationOnlyTest, authWithAccessIdFailed) {
  shared_ptr<supla_client> client;
  TCS_ClientAuthorizationDetails auth = {};

  auth.GUID[0] = 1;
  auth.AuthKey[0] = 2;
  auth.AccessID = 10;
  snprintf(auth.AccessIDpwd, sizeof(auth.AccessIDpwd), "%s", "abcd");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(client_dao, access_id_auth(10, StrEq("abcd"), NotNull(),
                                         NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int access_id, char *access_id_pwd, int *user_id,
                   bool *is_enabled, bool *is_active) {
        *user_id = 15;
        *is_enabled = true;
        *is_active = true;
        return false;
      });

  rc.authenticate(client, &auth, &srpcAdapter, &dba, nullptr, &client_dao,
                  true);

  EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, rc.get_result_code());
}

TEST_F(ClientAuthorizationOnlyTest, authWithAccessIdSuccessfull) {
  shared_ptr<supla_client> client;
  TCS_ClientAuthorizationDetails auth = {};

  auth.GUID[0] = 1;
  auth.AuthKey[0] = 2;
  auth.AccessID = 10;
  snprintf(auth.AccessIDpwd, sizeof(auth.AccessIDpwd), "%s", "abcd");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, is_connected).Times(1).WillOnce(Return(true));
  EXPECT_CALL(dba, start_transaction).Times(0);
  EXPECT_CALL(dba, rollback).Times(0);
  EXPECT_CALL(dba, commit).Times(0);
  EXPECT_CALL(dba, disconnect).Times(0);

  EXPECT_CALL(client_dao, update_client).Times(0);

  EXPECT_CALL(client_dao, access_id_auth(10, StrEq("abcd"), NotNull(),
                                         NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int access_id, char *access_id_pwd, int *user_id,
                   bool *is_enabled, bool *is_active) {
        *user_id = 15;
        *is_enabled = true;
        *is_active = true;
        return true;
      });

  EXPECT_CALL(client_dao, get_client_id(15, NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(client_dao, get_client_variables(987, NotNull(), NotNull(),
                                               NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int client_id, bool *client_enabled, int *access_id,
                   bool *accessid_enabled, bool *accessid_active) {
        *accessid_enabled = true;
        *accessid_active = true;
        return true;
      });

  rc.authenticate(client, &auth, &srpcAdapter, &dba, nullptr, &client_dao,
                  true);

  EXPECT_EQ(SUPLA_RESULTCODE_TRUE, rc.get_result_code());
}

TEST_F(ClientAuthorizationOnlyTest, authWithEmailFailed) {
  shared_ptr<supla_client> client;
  TCS_ClientAuthorizationDetails auth = {};

  auth.GUID[0] = 1;
  auth.AuthKey[0] = 2;
  snprintf(auth.Email, sizeof(auth.Email), "%s", "ted@atari.com");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("ted@atari.com")))
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
      .WillOnce([](int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                   bool *is_null) { return false; });

  rc.authenticate(client, &auth, &srpcAdapter, &dba, nullptr, &client_dao,
                  true);

  EXPECT_EQ(SUPLA_RESULTCODE_BAD_CREDENTIALS, rc.get_result_code());
}

TEST_F(ClientAuthorizationOnlyTest, authWithEmailSuccessfull) {
  shared_ptr<supla_client> client;
  TCS_ClientAuthorizationDetails auth = {};

  auth.GUID[0] = 1;
  auth.AuthKey[0] = 2;
  snprintf(auth.Email, sizeof(auth.Email), "%s", "ted@atari.com");

  EXPECT_CALL(dba, connect).Times(1).WillOnce(Return(true));

  EXPECT_CALL(rc, get_user_id_by_email(StrEq("ted@atari.com")))
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
            *is_null = true;
            return true;
          });

  EXPECT_CALL(client_dao, get_client_id(25, NotNull()))
      .Times(1)
      .WillOnce(Return(987));

  EXPECT_CALL(client_dao, get_client_variables(987, NotNull(), NotNull(),
                                               NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int client_id, bool *client_enabled, int *access_id,
                   bool *accessid_enabled, bool *accessid_active) {
        *accessid_enabled = true;
        *accessid_active = true;
        *access_id = 75;
        return true;
      });

  rc.authenticate(client, &auth, &srpcAdapter, &dba, nullptr, &client_dao,
                  true);

  EXPECT_EQ(SUPLA_RESULTCODE_TRUE, rc.get_result_code());
}

} /* namespace testing */
