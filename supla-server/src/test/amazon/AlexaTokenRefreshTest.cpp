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

#include "AlexaTokenRefreshTest.h"

#include <memory>

#include "amazon/alexa_change_report_request.h"
#include "amazon/alexa_client.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "doubles/http/CurlAdapterMock.h"
#include "http/asynctask_http_thread_bucket.h"
#include "log.h"

namespace testing {

using std::string;

void AlexaTokenRefreshTest::SetUp(void) { AsyncTaskTest::SetUp(); }

TEST_F(AlexaTokenRefreshTest, skillDisabledException) {
  CurlAdapterMock *curlAdapter = new CurlAdapterMock();
  ChannelPropertyGetterMock *propertyGetter = new ChannelPropertyGetterMock();

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(400));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, perform).WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .WillRepeatedly([](int instance_id, string *request_result) {
        request_result->append(
            "{\"payload\": {\"code\": \"SKILL_DISABLED_EXCEPTION\"}}");
      });

  EXPECT_CALL(credentials, update).Times(0);

  EXPECT_CALL(credentials, remove).Times(1);

  supla_alexa_change_report_request *request =
      new supla_alexa_change_report_request(supla_caller(ctDevice), 1, 2, 567,
                                            queue, pool, propertyGetter,
                                            &credentials);
  request->set_delay_usec(0);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(AlexaTokenRefreshTest, refresh) {
  CurlAdapterMock curlAdapter;
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(credentials, get_set_time).WillRepeatedly([]() {
    struct timeval time = {};
    return time;
  });

  EXPECT_CALL(credentials, refresh_lock).Times(1);

  EXPECT_CALL(
      curlAdapter,
      set_opt_url(
          Eq(0), StrEq("https://xatgh8yc1j.execute-api.eu-west-1.amazonaws.com/"
                       "default/amazonRefreshTokenBridge")))
      .Times(1);

  EXPECT_CALL(credentials, get_refresh_token)
      .WillRepeatedly(Return("MyRefreshTokenXYZ"));

  const char postPayload1[] = "{\"refresh_token\":\"MyRefreshTokenXYZ\"}";

  EXPECT_CALL(curlAdapter, set_opt_post_fields(Eq(0), StrEq(postPayload1)))
      .Times(1);

  EXPECT_CALL(curlAdapter, get_response_code).WillRepeatedly(Return(200));

  EXPECT_CALL(curlAdapter, set_opt_write_data)
      .Times(1)
      .WillOnce([](int instance_id, string *request_result) {
        *request_result =
            "{\"access_token\": \"newAccessToken\", \"expires_in\": "
            "12345,\"refresh_token\": \"newRefreshToken\"}";
      });

  EXPECT_CALL(credentials, update(StrEq("newAccessToken"),
                                  StrEq("newRefreshToken"), Eq(12345)))
      .Times(1);

  EXPECT_CALL(credentials, refresh_unlock).Times(1);

  supla_alexa_client client(1, &curlAdapter, &credentials, "", "", "");
  client.refresh_token();
}

TEST_F(AlexaTokenRefreshTest, theTokenHasChangedInTheMeantime) {
  CurlAdapterMock curlAdapter;
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

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

  EXPECT_CALL(credentials, refresh_unlock).Times(1);

  supla_alexa_client client(1, &curlAdapter, &credentials, "", "", "");
  client.refresh_token();
}

}  // namespace testing
