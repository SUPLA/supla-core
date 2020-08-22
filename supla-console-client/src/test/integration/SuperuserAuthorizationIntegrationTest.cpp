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

#include "SuperuserAuthorizationIntegrationTest.h"
#include "log.h"
namespace testing {

SuperuserAuthorizationIntegrationTest::SuperuserAuthorizationIntegrationTest() {
  exceptedAuthorizationResultCode = 0;
}

SuperuserAuthorizationIntegrationTest::
    ~SuperuserAuthorizationIntegrationTest() {}

void SuperuserAuthorizationIntegrationTest::onSuperuserAuthorizationResult(
    bool authorized, int code) {
  ASSERT_EQ(code, exceptedAuthorizationResultCode);
  cancelIteration();
}

TEST_F(SuperuserAuthorizationIntegrationTest, AuthorizeWithBadEmail) {
  char email[] = "test.supla.org";
  char password[] = "supla!test";
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_superuser_authorization_request(sclient, email, password),
      0);

  exceptedAuthorizationResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  iterateUntilDefaultTimeout();
}

TEST_F(SuperuserAuthorizationIntegrationTest, AuthorizeWithBadPassword) {
  char email[] = "test@supla.org";
  char password[] = "supla@test";
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_superuser_authorization_request(sclient, email, password),
      0);

  exceptedAuthorizationResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  iterateUntilDefaultTimeout();
}

TEST_F(SuperuserAuthorizationIntegrationTest, AuthorizeWithCorrectCredentials) {
  char email[] = "test@supla.org";
  char password[] = "supla!test";
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_superuser_authorization_request(sclient, email, password),
      0);

  exceptedAuthorizationResultCode = SUPLA_RESULTCODE_AUTHORIZED;
  iterateUntilDefaultTimeout();
}

TEST_F(SuperuserAuthorizationIntegrationTest,
       ReauthorizationWithIncorrectPassword) {
  char email[] = "test@supla.org";
  char password[] = "supla!test";
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_superuser_authorization_request(sclient, email, password),
      0);

  exceptedAuthorizationResultCode = SUPLA_RESULTCODE_AUTHORIZED;
  iterateUntilDefaultTimeout();

  password[0] = '!';

  ASSERT_GT(
      supla_client_superuser_authorization_request(sclient, email, password),
      0);

  exceptedAuthorizationResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  iterateUntilDefaultTimeout();
}

TEST_F(SuperuserAuthorizationIntegrationTest, GetResultWithoutAuthorization) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_get_superuser_authorization_result(sclient), 0);

  exceptedAuthorizationResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  iterateUntilDefaultTimeout();
}

TEST_F(SuperuserAuthorizationIntegrationTest,
       GetResultAfterCorrectAuthorization) {
  char email[] = "test@supla.org";
  char password[] = "supla!test";
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_superuser_authorization_request(sclient, email, password),
      0);

  exceptedAuthorizationResultCode = SUPLA_RESULTCODE_AUTHORIZED;
  iterateUntilDefaultTimeout();

  ASSERT_GT(supla_client_get_superuser_authorization_result(sclient), 0);

  iterateUntilDefaultTimeout();
}

} /* namespace testing */
