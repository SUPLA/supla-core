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

#include "OAuthIntegrationTest.h"
#include "log.h"
#include "tools.h"

namespace testing {

OAuthIntegrationTest::OAuthIntegrationTest() {
  memset(&token, 0, sizeof(TSC_SetRegistrationEnabledResult));
}

void OAuthIntegrationTest::SetUp() {
  ProperlyRegistered::SetUp();
  runSqlScript("DeleteOAuthClients.sql");
}

OAuthIntegrationTest::~OAuthIntegrationTest() {}

void OAuthIntegrationTest::onOAuthTokenRequestResult(
    TSC_OAuthTokenRequestResult *result) {
  ASSERT_FALSE(result == NULL);
  memcpy(&token, result, sizeof(TSC_OAuthTokenRequestResult));
  cancelIteration();
}

TEST_F(OAuthIntegrationTest, OAuthTokenRequest) {
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_oauth_token_request(sclient), 0);
  iterateUntilDefaultTimeout();

  ASSERT_EQ(token.ResultCode, SUPLA_OAUTH_RESULTCODE_SUCCESS);
  ASSERT_EQ(token.Token.ExpiresIn, (unsigned int)300);

  char *addr = NULL;
  int addr_len = 0;

  int len = strnlen(token.Token.Token, SUPLA_OAUTH_TOKEN_MAXSIZE);
  for (int a = 0; a < len; a++) {
    if (token.Token.Token[a] == '.' && a < len - 1) {
      addr = st_openssl_base64_decode(&token.Token.Token[a + 1], len - a - 1,
                                      &addr_len);
      break;
    }
  }

  char https[] = "https://";

  ASSERT_FALSE(addr == NULL);
  ASSERT_GT(addr_len, 0);

  ASSERT_EQ(memcmp(addr, https, strlen(https)), 0);

  free(addr);
}

} /* namespace testing */
