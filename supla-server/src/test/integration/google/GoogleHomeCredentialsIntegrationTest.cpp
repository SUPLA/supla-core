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

#include "GoogleHomeCredentialsIntegrationTest.h"

#include "google/google_home_credentials.h"
#include "log.h"
#include "user.h"

namespace testing {

using std::string;
using std::vector;

TEST_F(GoogleHomeCredentialsIntegrationTest, defaults) {
  supla_google_home_credentials credentials;

  EXPECT_EQ(credentials.get_access_token(), "");
  EXPECT_EQ(credentials.get_refresh_token(), "");
  EXPECT_FALSE(credentials.is_access_token_exists());
  EXPECT_FALSE(credentials.is_refresh_token_exists());
  EXPECT_LT(credentials.expires_in(), 0);
}

TEST_F(GoogleHomeCredentialsIntegrationTest, load) {
  supla_user::user_free();
  supla_user::init();

  initTestDatabase();

  supla_google_home_credentials credentials(supla_user::find(2, true));

  EXPECT_FALSE(credentials.is_access_token_exists());

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

  runSqlScript("AddGoogleHomeCredentials.sql");

  credentials.load();

  EXPECT_EQ(
      credentials.get_access_token(),
      "11198ab7d5d5fa5db8f53bf03556a4018909b8b5d3ee9628014ef9e62206f7024eedebc2"
      "79cd21af0dd34dc2766ebe2cdb34056340269a0e13b6a5415f2c18ad86df0c17b48ebd54"
      "f6e516011ad937c81c4220887d028a66d8b4316dcc5e57651775629f");
  EXPECT_TRUE(credentials.is_access_token_exists());
}

TEST_F(GoogleHomeCredentialsIntegrationTest, excludeChannels) {
  supla_user::user_free();
  supla_user::init();

  supla_google_home_credentials credentials(supla_user::find(2, true));

  EXPECT_FALSE(credentials.is_channel_excluded(123));
  EXPECT_FALSE(credentials.is_channel_excluded(456));

  credentials.exclude_channel(678);

  EXPECT_FALSE(credentials.is_channel_excluded(123));
  EXPECT_FALSE(credentials.is_channel_excluded(456));
  EXPECT_TRUE(credentials.is_channel_excluded(678));

  credentials.exclude_channel(123);
  credentials.exclude_channel(456);

  EXPECT_TRUE(credentials.is_channel_excluded(123));
  EXPECT_TRUE(credentials.is_channel_excluded(456));
  EXPECT_TRUE(credentials.is_channel_excluded(678));

  credentials.load();

  EXPECT_FALSE(credentials.is_channel_excluded(123));
  EXPECT_FALSE(credentials.is_channel_excluded(456));
  EXPECT_FALSE(credentials.is_channel_excluded(678));
}

} /* namespace testing */
