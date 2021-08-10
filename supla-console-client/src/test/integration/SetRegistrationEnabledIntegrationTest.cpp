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

#include "SetRegistrationEnabledIntegrationTest.h"
#include "log.h"

namespace testing {

SetRegistrationEnabledIntegrationTest::SetRegistrationEnabledIntegrationTest() {
  expectedResultCode = 0;
  memset(&regEnabled, 0, sizeof(TSDC_RegistrationEnabled));
}

SetRegistrationEnabledIntegrationTest::
    ~SetRegistrationEnabledIntegrationTest() {}

void SetRegistrationEnabledIntegrationTest::onRegistrationEnabled(
    TSDC_RegistrationEnabled *reg_enabled) {
  ASSERT_FALSE(reg_enabled == NULL);
  memcpy(&regEnabled, reg_enabled, sizeof(TSDC_RegistrationEnabled));
  cancelIteration();
}

void SetRegistrationEnabledIntegrationTest::onSetRegistrationEnabledResult(
    TSC_SetRegistrationEnabledResult *result) {
  ASSERT_FALSE(result == NULL);
  ASSERT_EQ(result->ResultCode, expectedResultCode);
  cancelIteration();
}

TEST_F(SetRegistrationEnabledIntegrationTest,
       SetRegistrationEnabledByUnauthorizedClient) {
  expectedResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_set_registration_enabled(sclient, 30, 90), 0);
  iterateUntilDefaultTimeout();
}

TEST_F(SetRegistrationEnabledIntegrationTest, GetRegistrationEnabled) {
  regEnabled.client_timestamp = 9999;
  regEnabled.iodevice_timestamp = 8888;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_get_registration_enabled(sclient), 0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(regEnabled.client_timestamp, (unsigned int)0);
  ASSERT_EQ(regEnabled.iodevice_timestamp, (unsigned int)0);
}

TEST_F(SetRegistrationEnabledIntegrationTest, SetRegistrationEnabled) {
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  ASSERT_GT(supla_client_set_registration_enabled(sclient, 100, 200), 0);
  iterateUntilDefaultTimeout();

  ASSERT_GT(supla_client_get_registration_enabled(sclient), 0);
  iterateUntilDefaultTimeout();

  struct timeval now;
  gettimeofday(&now, NULL);

  unsigned int timediff = abs(regEnabled.client_timestamp - now.tv_sec);
  ASSERT_TRUE(timediff >= 199 && timediff <= 201);

  timediff = abs(regEnabled.iodevice_timestamp - now.tv_sec);
  ASSERT_TRUE(timediff >= 99 && timediff <= 101);
}

TEST_F(SetRegistrationEnabledIntegrationTest, SubZeroValueTest) {
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  ASSERT_GT(supla_client_set_registration_enabled(sclient, 100, 200), 0);
  iterateUntilDefaultTimeout();

  ASSERT_GT(supla_client_get_registration_enabled(sclient), 0);
  iterateUntilDefaultTimeout();

  struct timeval now;
  gettimeofday(&now, NULL);

  unsigned int timediff = abs(regEnabled.iodevice_timestamp - now.tv_sec);
  ASSERT_TRUE(timediff >= 99 && timediff <= 101);

  timediff = abs(regEnabled.client_timestamp - now.tv_sec);
  ASSERT_TRUE(timediff >= 199 && timediff <= 201);


  ASSERT_GT(supla_client_set_registration_enabled(sclient, -1, 300), 0);
  iterateUntilDefaultTimeout();

  ASSERT_GT(supla_client_get_registration_enabled(sclient), 0);
  iterateUntilDefaultTimeout();

  gettimeofday(&now, NULL);

  timediff = abs(regEnabled.iodevice_timestamp - now.tv_sec);
  ASSERT_TRUE(timediff >= 99 && timediff <= 101);

  timediff = abs(regEnabled.client_timestamp - now.tv_sec);
  ASSERT_TRUE(timediff >= 299 && timediff <= 301);

  ASSERT_GT(supla_client_set_registration_enabled(sclient, 400, -1), 0);
  iterateUntilDefaultTimeout();

  ASSERT_GT(supla_client_get_registration_enabled(sclient), 0);
  iterateUntilDefaultTimeout();

  gettimeofday(&now, NULL);

  timediff = abs(regEnabled.iodevice_timestamp - now.tv_sec);

  ASSERT_TRUE(timediff >= 399 && timediff <= 401);

  timediff = abs(regEnabled.client_timestamp - now.tv_sec);
  ASSERT_TRUE(timediff >= 299 && timediff <= 301);
}

} /* namespace testing */
