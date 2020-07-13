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

#include "RegistrationIntegrationTest.h"
#include "log.h"

namespace testing {

RegistrationIntegrationTest::RegistrationIntegrationTest() {
  ProtocolVersion = SUPLA_PROTO_VERSION;
  expectedRegistrationErrorCode = 0;
  sslEnabled = true;
  fillArrayWithOrdinalNumbers(GUID, SUPLA_GUID_SIZE, 0);
  fillArrayWithOrdinalNumbers(AuthKey, SUPLA_AUTHKEY_SIZE, 0);

  snprintf(Email, SUPLA_EMAIL_MAXSIZE, TESTUSER_EMAIL);
  AccessID = 0;
  memset(AccessIDpwd, 0, SUPLA_ACCESSID_PWD_MAXSIZE);
  memset(Password, 0, Password[SUPLA_PASSWORD_MAXSIZE]);
}

RegistrationIntegrationTest::~RegistrationIntegrationTest() {}

void RegistrationIntegrationTest::beforeClientInit(TSuplaClientCfg *scc) {
  scc->protocol_version = ProtocolVersion;
  scc->ssl_enabled = sslEnabled;
  memcpy(scc->clientGUID, GUID, SUPLA_GUID_SIZE);
  memcpy(scc->AuthKey, AuthKey, SUPLA_AUTHKEY_SIZE);
  memcpy(scc->Email, Email, SUPLA_EMAIL_MAXSIZE);
  memcpy(scc->Password, Password, SUPLA_PASSWORD_MAXSIZE);
  scc->AccessID = AccessID;
  memcpy(scc->AccessIDpwd, AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE);
}

void RegistrationIntegrationTest::onRegistered(
    TSC_SuplaRegisterClientResult_B *result) {
  IntegrationTest::onRegistered(result);
  ASSERT_EQ(expectedRegistrationErrorCode, 0);
  cancelIteration();
}

void RegistrationIntegrationTest::onRegistrationError(int code) {
  ASSERT_EQ(code, expectedRegistrationErrorCode);
  cancelIteration();
}

void RegistrationIntegrationTest::onChannelFunctionSetResult(
    TSC_SetChannelFunctionResult *result) {
  ASSERT_FALSE(result == NULL);
  ASSERT_EQ(result->ResultCode, SUPLA_RESULTCODE_UNAUTHORIZED);
  cancelIteration();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithBadGUID) {
  memset(GUID, 0, SUPLA_GUID_SIZE);
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_GUID_ERROR;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithBadGUID_NoSSL) {
  memset(GUID, 0, SUPLA_GUID_SIZE);
  sslEnabled = false;
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_GUID_ERROR;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithBadAuthKey) {
  memset(AuthKey, 0, SUPLA_AUTHKEY_SIZE);
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_AUTHKEY_ERROR;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithModifiedAuthKey) {
  AuthKey[0] = 50;
  initTestDatabase();
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_BAD_CREDENTIALS;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithBadCredentials_Email) {
  snprintf(Email, SUPLA_EMAIL_MAXSIZE, "noneistent@supla.org");
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_BAD_CREDENTIALS;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithBadCredentials_AccessID) {
  Email[0] = 0;
  AccessID = 1;
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_BAD_CREDENTIALS;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest,
       RegistrationWhenRegistrationIsDisabledAndThereIsNoAnyClientsRegistered) {
  initTestDatabase();
  runSqlScript("RemoveTestClient.sql");
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_REGISTRATION_DISABLED;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest,
       RegistrationWhenRegistrationIsEnabledAndThereIsNoAnyClientsRegistered) {
  initTestDatabase();
  runSqlScript("RemoveTestClient.sql");
  runSqlScript("EnableClientRegistrationForTestUser.sql");
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest,
       RegistrationAsSecondClientWhenRegistrationIsEnabled) {
  GUID[0] = 10;
  initTestDatabase();
  runSqlScript("EnableClientRegistrationForTestUser.sql");
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWhenClientIsDisabled) {
  initTestDatabase();
  runSqlScript("DisableTestClient.sql");
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_CLIENT_DISABLED;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithSuccess) {
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithSuccess_NoSSL) {
  sslEnabled = false;
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithSuccess_ProtocolVersion12) {
  ProtocolVersion = 12;
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithSuccess_ProtocolVersion11) {
  ProtocolVersion = 11;
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithSuccess_ProtocolVersion7) {
  ProtocolVersion = 7;
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWithEmail_ProtocolVersion6) {
  ProtocolVersion = 6;
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_BAD_CREDENTIALS;
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationUsingAccessID) {
  Email[0] = 0;
  AccessID = 2;
  snprintf(AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, "3311dbb5");
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest,
       RegistrationUsingAccessID_ProtocolVersion1) {
  ProtocolVersion = 1;
  Email[0] = 0;
  AccessID = 2;
  snprintf(AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, "3311dbb5");
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest, RegistrationWhenClientLimitIsExceeded) {
  GUID[0] = 10;
  initTestDatabase();
  runSqlScript("EnableClientRegistrationForTestUser.sql");
  runSqlScript("SetClientLimitToOne.sql");
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED;
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest,
       RegAsSecondClientWhenRegIsDisabledWithSuperuserIncorrectCredentials) {
  GUID[0] = 10;
  snprintf(Password, SUPLA_PASSWORD_MAXSIZE, "supla@test");
  expectedRegistrationErrorCode = SUPLA_RESULTCODE_BAD_CREDENTIALS;
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest,
       RegAsSecondClientWhenRegIsDisabledWithSuperuserCredentials) {
  GUID[0] = 10;
  snprintf(Password, SUPLA_PASSWORD_MAXSIZE, "%s", TESTUSER_PASSWD);
  initTestDatabase();
  iterateUntilDefaultTimeout();
}

TEST_F(RegistrationIntegrationTest,
       AuthorizationShouldBeRevokedAfterRegistration) {
  GUID[0] = 10;
  snprintf(Password, SUPLA_PASSWORD_MAXSIZE, "%s", TESTUSER_PASSWD);

  initTestDatabase();
  iterateUntilDefaultTimeout();

  // There is no function to check if the superuser is authored, so use one of
  // the functions that checks the authorization

  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_channel_function(sclient, 100000,
                                              SUPLA_CHANNELFNC_POWERSWITCH),
            0);

  iterateUntilDefaultTimeout();
}

} /* namespace testing */
