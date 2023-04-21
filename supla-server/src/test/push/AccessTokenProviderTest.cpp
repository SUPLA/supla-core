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

#include "AccessTokenProviderTest.h"

#include <atomic>

#include "doubles/http/CurlAdapterMock.h"
#include "log.h"

using std::atomic_int;
using std::string;

namespace testing {

AccessTokenProviderTest::AccessTokenProviderTest(void) {
  curlAdapter = nullptr;
  provider = nullptr;
}

AccessTokenProviderTest::~AccessTokenProviderTest(void) {}

void AccessTokenProviderTest::SetUp(void) {
  Test::SetUp();
  curlAdapter = new CurlAdapterMock();
  provider = new AccessTokenProviderMock(curlAdapter);

  ON_CALL(*provider, min_secs_between_refresh_attempts)
      .WillByDefault(Return(2));
  ON_CALL(*provider, refresh_time_margin_secs).WillByDefault(Return(0));
  ON_CALL(*provider, service_tick_time_usec).WillByDefault(Return(1000000));
  ON_CALL(*curlAdapter, perform).WillByDefault(Return(true));
  ON_CALL(*curlAdapter, get_response_code).WillByDefault(Return(200));
}

void AccessTokenProviderTest::TearDown(void) {
  delete provider;
  Test::TearDown();
}

TEST_F(AccessTokenProviderTest, defaults) {
  EXPECT_FALSE(provider->is_service_running());
  EXPECT_FALSE(provider->is_any_token_available());
}

TEST_F(AccessTokenProviderTest, refreshThrottling) {
  int n = 1;
  int a = 0;

  EXPECT_CALL(*curlAdapter, reset).Times(2);
  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .Times(2)
      .WillRepeatedly([&n](string *request_result) {
        string token = "abcd" + std::to_string(n);
        n++;
        *request_result =
            "{\"android\":{\"0\":{\"token\":\"" + token +
            "\",\"url\":\"https://push.supla.org\",\"expires_in\":1}}}";
      });

  for (a = 0; a < 10; a++) {
    provider->refresh();
  }

  supla_pn_gateway_access_token token =
      provider->get_token(platform_android, 0);
  EXPECT_EQ(token.get_token(), "abcd1");
  EXPECT_TRUE(token.is_valid());

  usleep(2000000);

  for (a = 0; a < 10; a++) {
    provider->refresh();
  }

  token = provider->get_token(platform_android, 0);

  EXPECT_EQ(token.get_token(), "abcd2");
  EXPECT_TRUE(token.is_valid());
  EXPECT_EQ(provider->get_token_count(), 1);
}

TEST_F(AccessTokenProviderTest, refreshByServiceWithShortExpirationPeriod) {
  int n = 0;
  EXPECT_CALL(*provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .WillRepeatedly([&n](string *request_result) {
        string token = "abcd" + std::to_string(n);
        n++;
        *request_result =
            "{\"android\":{\"0\":{\"token\":\"" + token +
            "\",\"url\":\"https://push.supla.org\",\"expires_in\":1}}}";
      });

  EXPECT_FALSE(provider->is_service_running());
  provider->start_service();
  EXPECT_TRUE(provider->is_service_running());
  usleep(2500000);
  provider->stop_service();
  EXPECT_FALSE(provider->is_service_running());
  EXPECT_GE(n, 3);
  EXPECT_LE(n, 4);
}

TEST_F(AccessTokenProviderTest, refreshByServiceWithLongExpirationPeriod) {
  EXPECT_CALL(*provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .WillOnce([](string *request_result) {
        *request_result =
            "{\"android\":{\"0\":{\"token\":\"123\",\"url\":\"https://"
            "push.supla.org\",\"expires_in\":3600}}}";
      });

  provider->start_service();
  EXPECT_TRUE(provider->is_service_running());
  usleep(2500000);
  provider->stop_service();
  EXPECT_FALSE(provider->is_service_running());
  EXPECT_EQ(provider->get_token(platform_android, 0).get_token(), "123");
}

TEST_F(AccessTokenProviderTest, nextRefreshFailed) {
  int n = 0;

  EXPECT_CALL(*provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .WillRepeatedly([&n](string *request_result) {
        n++;
        if (n == 1) {
          *request_result =
              "{\"android\":{\"0\":{\"token\":\"1\",\"url\":\"https://"
              "push.supla.org\",\"expires_in\":1}}}";
        }
      });

  provider->start_service();
  EXPECT_TRUE(provider->is_service_running());
  usleep(2500000);
  provider->stop_service();
  EXPECT_FALSE(provider->is_service_running());
  EXPECT_GE(n, 3);
  EXPECT_LE(n, 4);
  EXPECT_EQ(provider->get_token(platform_android, 0).get_token(), "1");
}

TEST_F(AccessTokenProviderTest, timeMargin) {
  int n = 0;

  EXPECT_CALL(*provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));

  EXPECT_CALL(*provider, refresh_time_margin_secs).WillRepeatedly(Return(98));

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .WillRepeatedly([&n](string *request_result) {
        string token = std::to_string(n);
        n++;
        *request_result =
            "{\"android\":{\"0\":{\"token\":\"" + token +
            "\",\"url\":\"https://push.supla.org\",\"expires_in\":100}}}";
      });

  provider->start_service();
  EXPECT_TRUE(provider->is_service_running());
  usleep(2500000);
  provider->stop_service();
  EXPECT_FALSE(provider->is_service_running());
  EXPECT_GE(n, 2);
  EXPECT_LE(n, 3);
}

TEST_F(AccessTokenProviderTest, multipleTokensAndOneSoonExpired) {
  atomic_int n{0};

  EXPECT_CALL(*provider, min_secs_between_refresh_attempts)
      .WillRepeatedly(Return(0));

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .Times(2)
      .WillRepeatedly([&n](string *request_result) {
        n++;
        if (n.load() == 1) {
          *request_result =
              "{\"android\":{\"0\":{\"token\":\"abcd1\",\"expires_in\":3600,"
              "\"url\":\"https://"
              "push1.supla.org\"},\"1\":{\"token\":\"abcd2\",\"expires_in\":"
              "3700,\"url\":\"https://"
              "push2.supla.org\"},\"2\":{\"token\":\"abcd3\",\"expires_in\":"
              "3800,\"url\":\"https://"
              "push3.supla.org\"}},\"ios\":{\"0\":{\"token\":\"abcd4\","
              "\"expires_in\":1,\"url\":\"https://"
              "push4.supla.org\"},\"1\":{\"token\":\"abcd5\",\"expires_in\":"
              "4000,\"url\":\"https://"
              "push5.supla.org\"},\"2\":{\"token\":\"abcd6\",\"expires_in\":"
              "4100,\"url\":\"https://push6.supla.org\"}}}";
        } else {
          *request_result =
              "{\"ios\":{\"0\":{\"token\":\"XyZ\",\"url\":\"https://"
              "push.supla.org\",\"expires_in\":100}}}";
        }
      });

  EXPECT_FALSE(provider->is_service_running());
  provider->start_service();
  EXPECT_TRUE(provider->is_service_running());
  usleep(500000);

  EXPECT_EQ(n.load(), 1);
  EXPECT_EQ(provider->get_token_count(), 6);

  supla_pn_gateway_access_token token =
      provider->get_token(platform_android, 0);
  EXPECT_GE(token.get_expires_in(), 3599);
  EXPECT_LE(token.get_expires_in(), 3600);
  EXPECT_EQ(token.get_token(), "abcd1");
  EXPECT_EQ(token.get_url(), "https://push1.supla.org");

  token = provider->get_token(platform_android, 1);
  EXPECT_GE(token.get_expires_in(), 3699);
  EXPECT_LE(token.get_expires_in(), 3700);
  EXPECT_EQ(token.get_token(), "abcd2");
  EXPECT_EQ(token.get_url(), "https://push2.supla.org");

  token = provider->get_token(platform_android, 2);
  EXPECT_GE(token.get_expires_in(), 3799);
  EXPECT_LE(token.get_expires_in(), 3800);
  EXPECT_EQ(token.get_token(), "abcd3");
  EXPECT_EQ(token.get_url(), "https://push3.supla.org");

  token = provider->get_token(platform_ios, 0);
  EXPECT_GE(token.get_expires_in(), 0);
  EXPECT_LE(token.get_expires_in(), 1);
  EXPECT_EQ(token.get_token(), "abcd4");
  EXPECT_EQ(token.get_url(), "https://push4.supla.org");

  token = provider->get_token(platform_ios, 1);
  EXPECT_GE(token.get_expires_in(), 3999);
  EXPECT_LE(token.get_expires_in(), 4000);
  EXPECT_EQ(token.get_token(), "abcd5");
  EXPECT_EQ(token.get_url(), "https://push5.supla.org");

  token = provider->get_token(platform_ios, 2);
  EXPECT_GE(token.get_expires_in(), 3099);
  EXPECT_LE(token.get_expires_in(), 4100);
  EXPECT_EQ(token.get_token(), "abcd6");
  EXPECT_EQ(token.get_url(), "https://push6.supla.org");

  usleep(3000000);
  provider->stop_service();
  EXPECT_FALSE(provider->is_service_running());

  EXPECT_EQ(provider->get_token_count(), 6);

  token = provider->get_token(platform_android, 0);
  EXPECT_GE(token.get_expires_in(), 3595);
  EXPECT_LE(token.get_expires_in(), 3598);
  EXPECT_EQ(token.get_token(), "abcd1");
  EXPECT_EQ(token.get_url(), "https://push1.supla.org");

  token = provider->get_token(platform_android, 1);
  EXPECT_GE(token.get_expires_in(), 3695);
  EXPECT_LE(token.get_expires_in(), 3698);
  EXPECT_EQ(token.get_token(), "abcd2");
  EXPECT_EQ(token.get_url(), "https://push2.supla.org");

  token = provider->get_token(platform_android, 2);
  EXPECT_GE(token.get_expires_in(), 3795);
  EXPECT_LE(token.get_expires_in(), 3798);
  EXPECT_EQ(token.get_token(), "abcd3");
  EXPECT_EQ(token.get_url(), "https://push3.supla.org");

  token = provider->get_token(platform_ios, 0);
  EXPECT_GE(token.get_expires_in(), 95);
  EXPECT_LE(token.get_expires_in(), 100);
  EXPECT_EQ(token.get_token(), "XyZ");
  EXPECT_EQ(token.get_url(), "https://push.supla.org");

  token = provider->get_token(platform_ios, 1);
  EXPECT_GE(token.get_expires_in(), 3995);
  EXPECT_LE(token.get_expires_in(), 3998);
  EXPECT_EQ(token.get_token(), "abcd5");
  EXPECT_EQ(token.get_url(), "https://push5.supla.org");

  token = provider->get_token(platform_ios, 2);
  EXPECT_GE(token.get_expires_in(), 4095);
  EXPECT_LE(token.get_expires_in(), 4098);
  EXPECT_EQ(token.get_token(), "abcd6");
  EXPECT_EQ(token.get_url(), "https://push6.supla.org");
}

} /* namespace testing */
