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

#include "StateWebhookTokenRefreshTest.h"

#include "http/asynctask_http_thread_bucket.h"
#include "log.h"
#include "webhook/state_webhook_request.h"

namespace testing {

using std::string;

void StateWebhookTokenRefreshTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();
  propertyGetter = new ChannelPropertyGetterMock();

  EXPECT_CALL(credentials, get_url)
      .WillRepeatedly(Return("https://webhook.test.io/endpoint"));

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));
}

TEST_F(StateWebhookTokenRefreshTest, expired) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, expires_in).Times(1).WillOnce(Return(30));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(credentials, get_set_time).WillRepeatedly([]() {
    struct timeval time = {};
    return time;
  });

  EXPECT_CALL(credentials, refresh_lock).Times(1);

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .WillRepeatedly(Return("dc85740d-cb27-405b-9da3-e8be5c71ae5b"));

  EXPECT_CALL(credentials, get_refresh_token)
      .WillRepeatedly(Return("MyRefreshTokenXYZ"));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(2)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, set_opt_custom_request(StrEq("PUT"))).Times(1);

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://webhook.test.io/endpoint")))
      .Times(2);

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .Times(1)
      .WillOnce([](string *request_result) {
        *request_result =
            "{\"accessToken\": \"newAccessToken\", \"expiresIn\": "
            "12345,\"refreshToken\": \"newRefreshToken\"}";
      });

  const char postPayload1[] =
      "{\"refreshToken\":\"MyRefreshTokenXYZ\",\"userShortUniqueId\":"
      "\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\"}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(postPayload1))).Times(1);

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(200));

  EXPECT_CALL(credentials, update(StrEq("newAccessToken"),
                                  StrEq("newRefreshToken"), Eq(12345)))
      .Times(1);

  {
    InSequence s;
    EXPECT_CALL(credentials, refresh_unlock).Times(1);

    EXPECT_CALL(credentials, get_access_token)
        .WillRepeatedly(Return("newAccessToken"));
  }

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer newAccessToken")))
      .Times(1)
      .WillOnce(Return(true));

  const char postPayload2[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":567,\"channelFunction\":\"ACTION_TRIGGER\",\"timestamp\":"
      "1679859036,\"triggered_actions\":[\"HOLD\"]}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(postPayload2))).Times(1);

  EXPECT_CALL(*curlAdapter, reset).Times(2);

  EXPECT_CALL(*curlAdapter, perform).Times(2).WillRepeatedly(Return(true));

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 567, SUPLA_ACTION_CAP_HOLD, queue, pool,
      propertyGetter, &credentials);
  request->set_timestamp(1679859036);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(StateWebhookTokenRefreshTest, refreshTokenNotExists) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, expires_in).Times(1).WillOnce(Return(30));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .Times(1)
      .WillOnce(Return(false));

  EXPECT_CALL(credentials, get_set_time).Times(0);

  EXPECT_CALL(credentials, refresh_lock).Times(0);

  EXPECT_CALL(credentials, update).Times(0);

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 567, SUPLA_ACTION_CAP_HOLD, queue, pool,
      propertyGetter, &credentials);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(StateWebhookTokenRefreshTest, theTokenHasChangedInTheMeantime) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, expires_in).Times(1).WillOnce(Return(30));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .Times(1)
      .WillOnce(Return(true));

  struct timeval time = {};

  EXPECT_CALL(credentials, get_set_time).WillRepeatedly([&time]() {
    time.tv_usec++;
    return time;
  });

  EXPECT_CALL(credentials, refresh_lock).Times(1);

  EXPECT_CALL(credentials, get_refresh_token).Times(0);

  EXPECT_CALL(credentials, update).Times(0);

  {
    InSequence s;
    EXPECT_CALL(credentials, refresh_unlock).Times(1);

    EXPECT_CALL(credentials, get_access_token)
        .WillRepeatedly(Return("AccessTokenEFGH"));
  }

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(200));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://webhook.test.io/endpoint")))
      .Times(1);

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .Times(1)
      .WillOnce(Return("dc85740d-cb27-405b-9da3-e8be5c71ae5b"));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer AccessTokenEFGH")))
      .Times(1)
      .WillOnce(Return(true));

  const char postPayload2[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":567,\"channelFunction\":\"ACTION_TRIGGER\",\"timestamp\":"
      "1679859036,\"triggered_actions\":[\"HOLD\"]}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(postPayload2))).Times(1);

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 567, SUPLA_ACTION_CAP_HOLD, queue, pool,
      propertyGetter, &credentials);
  request->set_timestamp(1679859036);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(StateWebhookTokenRefreshTest, http403) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, expires_in).Times(1).WillOnce(Return(1000));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(403));

  EXPECT_CALL(*curlAdapter, perform).Times(2).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, remove).Times(1);

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 567, SUPLA_ACTION_CAP_HOLD, queue, pool,
      propertyGetter, &credentials);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

}  // namespace testing
