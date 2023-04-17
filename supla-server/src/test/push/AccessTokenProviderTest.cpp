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

#include "push/AccessTokenProviderTest.h"

#include "doubles/http/CurlAdapterMock.h"

namespace testing {

AccessTokenProviderTest::AccessTokenProviderTest(void) {}

AccessTokenProviderTest::~AccessTokenProviderTest(void) {}

void AccessTokenProviderTest::SetUp(void) {
  Test::SetUp();
  ON_CALL(provider, min_secs_between_refresh_attempts).WillByDefault(Return(2));
  ON_CALL(provider, refresh_time_margin_secs).WillByDefault(Return(0));
  ON_CALL(provider, service_tick_time_usec).WillByDefault(Return(1000000));
}

TEST_F(AccessTokenProviderTest, defaults) {
  EXPECT_FALSE(provider.is_service_running());
  EXPECT_FALSE(provider.is_token_valid());
  EXPECT_EQ(provider.get_token().size(), 0);
  EXPECT_EQ(provider.get_url().size(), 0);
}

TEST_F(AccessTokenProviderTest, refreshThrottling) {
  int a = 0;

  CurlAdapterMock curl;

  EXPECT_CALL(provider, new_token(IsNull(), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](supla_abstract_curl_adapter *curl_adapter,
                   std::string *token, int *expires_in_secs) {
        *expires_in_secs = 3600;
        *token = std::to_string(1);
        return true;
      });

  EXPECT_CALL(provider, new_token(Eq(&curl), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](supla_abstract_curl_adapter *curl_adapter,
                   std::string *token, int *expires_in_secs) {
        *expires_in_secs = 3600;
        *token = std::to_string(2);
        return true;
      });

  for (a = 0; a < 10; a++) {
    provider.refresh();
  }

  EXPECT_EQ(provider.get_token(), "1");
  EXPECT_TRUE(provider.is_token_valid());

  usleep(2000000);

  for (a = 0; a < 10; a++) {
    provider.refresh(&curl);
  }

  EXPECT_EQ(provider.get_token(), "2");
  EXPECT_TRUE(provider.is_token_valid());
}

TEST_F(AccessTokenProviderTest,
       refreshByServiceWithoutCurlAndWithShortExpirationPeriod) {
  int n = 0;
  EXPECT_CALL(provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));
  EXPECT_CALL(provider, new_token(IsNull(), NotNull(), NotNull()))
      .WillRepeatedly([&n](supla_abstract_curl_adapter *curl_adapter,
                           std::string *token, int *expires_in_secs) {
        n++;
        *expires_in_secs = 1;
        *token = std::to_string(n);

        return true;
      });

  provider.start_service();
  EXPECT_TRUE(provider.is_service_running());
  usleep(2500000);
  provider.stop_service();
  EXPECT_FALSE(provider.is_service_running());
  EXPECT_GE(n, 3);
  EXPECT_LE(n, 4);
}

TEST_F(AccessTokenProviderTest,
       refreshByServiceWithCurlAndLongExpirationPeriod) {
  CurlAdapterMock *curl = new CurlAdapterMock();
  AccessTokenProviderMock provider(curl);

  ON_CALL(provider, refresh_time_margin_secs)
      .WillByDefault(Return(this->provider.refresh_time_margin_secs()));
  ON_CALL(provider, service_tick_time_usec)
      .WillByDefault(Return(this->provider.service_tick_time_usec()));

  EXPECT_CALL(provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));
  EXPECT_CALL(provider, new_token(Eq(curl), NotNull(), NotNull()))
      .WillOnce([](supla_abstract_curl_adapter *curl_adapter,
                   std::string *token, int *expires_in_secs) {
        *expires_in_secs = 0;  // 0 == 10 years
        *token = "123";

        return true;
      });

  provider.start_service();
  EXPECT_TRUE(provider.is_service_running());
  usleep(2500000);
  provider.stop_service();
  EXPECT_FALSE(provider.is_service_running());
  EXPECT_EQ(provider.get_token(), "123");
}

TEST_F(AccessTokenProviderTest, refreshFailed) {
  int n = 0;
  EXPECT_CALL(provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));
  EXPECT_CALL(provider, new_token(IsNull(), NotNull(), NotNull()))
      .WillRepeatedly([&n](supla_abstract_curl_adapter *curl_adapter,
                           std::string *token, int *expires_in_secs) {
        n++;
        *expires_in_secs = n;
        *token = std::to_string(n);

        return n == 1;
      });

  provider.start_service();
  EXPECT_TRUE(provider.is_service_running());
  usleep(2500000);
  provider.stop_service();
  EXPECT_FALSE(provider.is_service_running());
  EXPECT_GE(n, 3);
  EXPECT_LE(n, 4);
  EXPECT_EQ(provider.get_token(), "1");
}

TEST_F(AccessTokenProviderTest, timeMargin) {
  int n = 0;
  EXPECT_CALL(provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));
  EXPECT_CALL(provider, refresh_time_margin_secs).WillRepeatedly(Return(98));
  EXPECT_CALL(provider, new_token(IsNull(), NotNull(), NotNull()))
      .WillRepeatedly([&n](supla_abstract_curl_adapter *curl_adapter,
                           std::string *token, int *expires_in_secs) {
        n++;
        *expires_in_secs = 100;
        *token = std::to_string(n);

        return true;
      });

  provider.start_service();
  EXPECT_TRUE(provider.is_service_running());
  usleep(2500000);
  provider.stop_service();
  EXPECT_FALSE(provider.is_service_running());
  EXPECT_GE(n, 2);
  EXPECT_LE(n, 3);
}

} /* namespace testing */
