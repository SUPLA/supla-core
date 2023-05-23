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

#include "AmazonAlexaCredentialsIntegrationTest.h"

#include "amazon/alexa_credentials.h"
#include "log.h"
#include "user.h"

namespace testing {

using std::string;
using std::vector;

TEST_F(AmazonAlexaCredentialsIntegrationTest, defaults) {
  supla_amazon_alexa_credentials credentials;

  EXPECT_EQ(credentials.get_access_token(), "");
  EXPECT_EQ(credentials.get_refresh_token(), "");
  EXPECT_FALSE(credentials.is_access_token_exists());
  EXPECT_FALSE(credentials.is_refresh_token_exists());
  EXPECT_LT(credentials.expires_in(), 0);
  EXPECT_EQ(credentials.get_region(), "");
}

TEST_F(AmazonAlexaCredentialsIntegrationTest, load) {
  supla_user::user_free();
  supla_user::init();

  initTestDatabase();

  supla_amazon_alexa_credentials credentials(supla_user::find(2, true));

  EXPECT_FALSE(credentials.is_access_token_exists());
  EXPECT_FALSE(credentials.is_refresh_token_exists());

  EXPECT_EQ(credentials.get_user_short_unique_id(),
            "6d4e39883a09d3baffe9e15b5fedc152");

  EXPECT_EQ(
      credentials.get_user_long_unique_id(),
      "090874ade7b4e23aef2330e81cd4784cc3badcb0b6df34c8a0be0d91ded0b5567fde8b36"
      "bf920c5a2e1ead39aa4494d3d262a1cc4a6a93225a15fec85265858f85936af20236cb7b"
      "50a8e9a420897c966cbe22f2e55275f033b6b2b10e2b5136bcb31fcb");

  EXPECT_EQ(credentials.get_user_id(), 2);

  credentials.load();

  EXPECT_FALSE(credentials.is_access_token_exists());
  EXPECT_FALSE(credentials.is_refresh_token_exists());

  runSqlScript("AddAlexaCredentials.sql");
  credentials.load();

  EXPECT_EQ(credentials.get_access_token(),
            "Atza|IwEBIPSGcmz4Lhlw8ixBwOt5rR1PDg1lByehicWqVuEU_O_"
            "P2OfI2iSPhJD1QDwUZNAKmgyFoktUmbDC2xwYUVjHK47q5OsWMENo06l6H2GnSY7AP"
            "yg5gYlthyVwz2VG1c-NlkGuBpu41yvifANlHDdGq0ssbYMXtowPm2CdrRb-"
            "bviWE2unYnhyzosyusZPadDXf5lG9FmcYOetyJrWOGXzF5iO0hNSpECPmJvsD0BQ3H"
            "KsKch7WAdn4746XVYiQgfPUtnDhIkVjZwMsJI6tE1ZRnRG");
  EXPECT_EQ(
      credentials.get_refresh_token(),
      "Atzr1IwEBIN_rv71MXWPAJWMj0IPnoWpR6QZugZoYnFDSpQ5pBxOwsTfe1PAzwR3Mo_"
      "VzaaER8YYLIrg8qXruJ0eNU2p3oQ6ptjPIEm8DevCl3hBOdePpe_"
      "xqQDRifAzNYbgHlL6nqyyHHRYDtdrfiSDo-lYpkEXdhy7VdW3sNHJfB0POKg-"
      "wTDr6XFryrji_BvCM0_dxS8YOClUH6AOzu0mKBEO93nvdXrlVXEFGbIr2MZ61-"
      "dZOan1bDeX2xmK-qflLeLCusl0yR6oM0StyAdpQYp6IiWwk");
  EXPECT_TRUE(credentials.is_access_token_exists());
  EXPECT_TRUE(credentials.is_refresh_token_exists());
  EXPECT_GT(credentials.expires_in(), 25);
  EXPECT_LE(credentials.expires_in(), 30);
  EXPECT_EQ(credentials.get_region(), "eu");
}

TEST_F(AmazonAlexaCredentialsIntegrationTest, update) {
  initTestDatabase();

  supla_amazon_alexa_credentials credentials(supla_user::find(2, true));

  credentials.load();

  EXPECT_FALSE(credentials.is_access_token_exists());
  EXPECT_FALSE(credentials.is_refresh_token_exists());

  runSqlScript("AddAlexaCredentials.sql");

  credentials.update("new_access_token", "new_refresh_token", 3600);

  EXPECT_EQ(credentials.get_access_token(), "new_access_token");
  EXPECT_EQ(credentials.get_refresh_token(), "new_refresh_token");

  EXPECT_GT(credentials.expires_in(), 3595);
  EXPECT_LE(credentials.expires_in(), 3600);

  string result;
  sqlQuery(
      "SELECT count(*) as count FROM supla_amazon_alexa WHERE user_id = 2 "
      "AND access_token = 'new_access_token' AND refresh_token = "
      "'new_refresh_token' AND TIMESTAMPDIFF(SECOND,UTC_TIMESTAMP(), "
      "expires_at) >= 3590",
      &result);

  EXPECT_EQ(result, "count\n1\n");
}

TEST_F(AmazonAlexaCredentialsIntegrationTest, remove) {
  initTestDatabase();
  supla_amazon_alexa_credentials credentials(supla_user::find(2, true));

  runSqlScript("AddAlexaCredentials.sql");

  credentials.load();

  EXPECT_TRUE(credentials.is_access_token_exists());
  EXPECT_TRUE(credentials.is_refresh_token_exists());

  credentials.remove();
  credentials.load();

  EXPECT_FALSE(credentials.is_access_token_exists());
  EXPECT_FALSE(credentials.is_refresh_token_exists());

  string result;
  sqlQuery(
      "SELECT count(*) as count FROM supla_state_webhooks WHERE user_id = 2 "
      "AND LENGTH(access_token) > 0",
      &result);

  EXPECT_EQ(result, "count\n0\n");
}

} /* namespace testing */
