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

#include "GoogleHomeSyncRequestTest.h"

#include <memory>

#include "google/google_home_sync_request.h"
#include "http/asynctask_http_thread_bucket.h"

namespace testing {

using std::string;

void GoogleHomeSyncRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();

  tokenProviderCurlAdapter = new CurlAdapterMock();
  tokenProvider = new AccessTokenProviderMock(tokenProviderCurlAdapter);

  ON_CALL(*tokenProviderCurlAdapter, perform).WillByDefault(Return(true));
  ON_CALL(*tokenProviderCurlAdapter, get_response_code)
      .WillByDefault(Return(200));

  ON_CALL(*tokenProviderCurlAdapter, set_opt_write_data)
      .WillByDefault([this](string *request_result) {
        *request_result =
            "{\"homegraph\":{\"0\":{\"token\":\"directTokenXyz\",\"expires_"
            "in\":3600}}}";
      });

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));
}

void GoogleHomeSyncRequestTest::TearDown(void) {
  if (tokenProvider) {
    delete tokenProvider;
    tokenProvider = nullptr;
  }

  if (tokenProviderCurlAdapter) {
    delete tokenProviderCurlAdapter;
    tokenProviderCurlAdapter = nullptr;
  }

  AsyncTaskTest::TearDown();
}

TEST_F(GoogleHomeSyncRequestTest, noAccessToken) {
  EXPECT_CALL(credentials, is_access_token_exists)
      .Times(1)
      .WillOnce(Return(false));

  supla_google_home_sync_request *request = new supla_google_home_sync_request(
      0, queue, pool, &credentials, tokenProvider);

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(GoogleHomeSyncRequestTest, syncSuccessful) {
  string expectedPayload =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"intent\":\"action.devices.SYNC\"}";

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_access_token)
      .WillOnce(Return("MyAccessTokenXYZ"));

  EXPECT_CALL(credentials, get_user_long_unique_id).WillOnce(Return("zxcvbnm"));

  EXPECT_CALL(*curlAdapter, set_opt_post_fields)
      .Times(1)
      .WillOnce([&expectedPayload](const char *fields) {
        expectedPayload.replace(14, 36, string(fields).substr(14, 36));
        EXPECT_TRUE(fields == expectedPayload);
      });

  EXPECT_CALL(*curlAdapter, reset).Times(1);

  EXPECT_CALL(
      *curlAdapter,
      set_opt_url(StrEq(
          "https://"
          "odokilkqoesh73zfznmiupey4a0uugaz.lambda-url.eu-west-1.on.aws/")))
      .Times(1);

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer MyAccessTokenXYZ")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(200));

  supla_google_home_sync_request *request = new supla_google_home_sync_request(
      0, queue, pool, &credentials, tokenProvider);

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(GoogleHomeSyncRequestTest, directSyncWithSuccess) {
  tokenProvider->refresh();

  string expectedPayload = "{\"agentUserId\":\"zxcvbnm\"}";

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_user_long_unique_id)
      .WillRepeatedly(Return("zxcvbnm"));

  EXPECT_CALL(*curlAdapter, set_opt_post_fields)
      .Times(1)
      .WillOnce([&expectedPayload](const char *fields) {
        expectedPayload.replace(14, 36, string(fields).substr(14, 36));
        EXPECT_TRUE(fields == expectedPayload);
      });

  EXPECT_CALL(*curlAdapter, reset).Times(1);

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq(
                  "https://homegraph.googleapis.com/v1/devices:requestSync")))
      .Times(1);

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer directTokenXyz")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(200));

  supla_google_home_sync_request *request = new supla_google_home_sync_request(
      0, queue, pool, &credentials, tokenProvider);

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

}  // namespace testing
