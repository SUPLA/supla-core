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

#include "AmazonAlexaCredentialsDaoIntegrationTest.h"

#include <string>

#include "amazon/alexa_credentials_dao.h"
#include "log.h"

using std::string;

namespace testing {

TEST_F(AmazonAlexaCredentialsDaoIntegrationTest, getCloudToken) {
  initTestDatabase();

  runSqlScript("DeleteOAuthAccessTokens.sql");
  runSqlScript("AddAlexaOAuthClient.sql");

  string result;
  sqlQuery("SELECT count(*) as count FROM supla_oauth_access_tokens", &result);

  EXPECT_EQ(result, "count\n0\n");

  supla_amazon_alexa_credentials_dao dao(&dba);
  string token1 = dao.get_cloud_access_token(2);
  string token2 = dao.get_cloud_access_token(2);

  EXPECT_FALSE(token1.empty());
  EXPECT_EQ(token1, token2);

  runSqlScript("ExpireOAuthTokens.sql");
  token2 = dao.get_cloud_access_token(2);

  EXPECT_NE(token1, token2);

  string expected_suffix = ".aHR0cHM6Ly9iZXRhLWNsb3VkLnN1cGxhLm9yZw==";
  ASSERT_TRUE(expected_suffix.length() < token1.length());
  EXPECT_EQ(token1.compare(token1.length() - expected_suffix.length(),
                           expected_suffix.length(), expected_suffix),
            0);
}

} /* namespace testing */
