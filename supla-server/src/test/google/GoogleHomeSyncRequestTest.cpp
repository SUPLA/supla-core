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

#include "google/GoogleHomeSyncRequestTest.h"

#include "google/google_home_sync_request2.h"
#include "http/asynctask_http_thread_bucket.h"

namespace testing {

using std::string;

void GoogleHomeSyncRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));
}

TEST_F(GoogleHomeSyncRequestTest, noAccessToken) {
  EXPECT_CALL(credentials, is_access_token_exists)
      .Times(1)
      .WillOnce(Return(false));

  supla_google_home_sync_request2 *request =
      new supla_google_home_sync_request2(supla_caller(ctUnknown), 0,
                                          ET_GOOGLE_HOME_SYNC_NEEDED, queue,
                                          pool, &credentials);

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

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq(
                  "https://2rxqysinpg.execute-api.eu-west-1.amazonaws.com/"
                  "default/googleHomeGraphBridge")))
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

  supla_google_home_sync_request2 *request =
      new supla_google_home_sync_request2(supla_caller(ctUnknown), 0,
                                          ET_GOOGLE_HOME_SYNC_NEEDED, queue,
                                          pool, &credentials);

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(GoogleHomeSyncRequestTest, http404) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, on_sync_40x_error).Times(1);

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(404));

  supla_google_home_sync_request2 *request =
      new supla_google_home_sync_request2(supla_caller(ctUnknown), 0,
                                          ET_GOOGLE_HOME_SYNC_NEEDED, queue,
                                          pool, &credentials);

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(GoogleHomeSyncRequestTest, http403) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, on_sync_40x_error).Times(1);

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(403));

  supla_google_home_sync_request2 *request =
      new supla_google_home_sync_request2(supla_caller(ctUnknown), 0,
                                          ET_GOOGLE_HOME_SYNC_NEEDED, queue,
                                          pool, &credentials);

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

}  // namespace testing
