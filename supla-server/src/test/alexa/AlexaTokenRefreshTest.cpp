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

#include "amazon/alexa_change_report_request.h"
#include "http/asynctask_http_thread_bucket.h"
#include "log.h"

namespace testing {

using std::string;

void AlexaTokenRefreshTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();
  propertyGetter = new ChannelPropertyGetterMock();

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));
}

void AlexaTokenRefreshTest::httpCodeTest(int code) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, expires_in).Times(1).WillOnce(Return(30));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(code));

  EXPECT_CALL(*curlAdapter, perform).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, update).Times(0);

  EXPECT_CALL(credentials, remove).Times(1);

  supla_alexa_change_report_request2 *request =
      new supla_alexa_change_report_request2(supla_caller(ctDevice), 1, 2, 567,
                                             queue, pool, propertyGetter,
                                             &credentials);
  request->set_delay_usec(0);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

void AlexaTokenRefreshTest::badRequestTest(const string &exception,
                                           bool removingExpected) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, expires_in).Times(1).WillOnce(Return(30));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(400));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, perform).WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, set_opt_write_data)
      .WillRepeatedly([exception](string *request_result) {
        request_result->append("{\"payload\": {\"code\": \"");
        request_result->append(exception);
        request_result->append("\"}}");
      });

  EXPECT_CALL(credentials, update).Times(0);

  EXPECT_CALL(credentials, remove).Times(removingExpected ? 1 : 0);

  supla_alexa_change_report_request2 *request =
      new supla_alexa_change_report_request2(supla_caller(ctDevice), 1, 2, 567,
                                             queue, pool, propertyGetter,
                                             &credentials);
  request->set_delay_usec(0);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(AlexaTokenRefreshTest, expired) {
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

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq(
                  "https://xatgh8yc1j.execute-api.eu-west-1.amazonaws.com/"
                  "default/amazonRefreshTokenBridge")))
      .Times(1);

  EXPECT_CALL(credentials, get_refresh_token)
      .WillRepeatedly(Return("MyRefreshTokenXYZ"));

  const char postPayload1[] = "{\"refresh_token\":\"MyRefreshTokenXYZ\"}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(postPayload1))).Times(1);

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(200));

  {
    InSequence s;
    EXPECT_CALL(*curlAdapter, set_opt_write_data)
        .Times(1)
        .WillOnce([](string *request_result) {
          *request_result =
              "{\"access_token\": \"newAccessToken\", \"expires_in\": "
              "12345,\"refresh_token\": \"newRefreshToken\"}";
        });

    EXPECT_CALL(*curlAdapter, set_opt_write_data).Times(1);
  }

  EXPECT_CALL(credentials, update(StrEq("newAccessToken"),
                                  StrEq("newRefreshToken"), Eq(12345)))
      .Times(1);

  EXPECT_CALL(credentials, refresh_unlock).Times(1);

  EXPECT_CALL(credentials, get_region).WillOnce(Return(""));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer AccessTokenXYZ")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://api.amazonalexa.com/v3/events")))
      .Times(1);

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("AccessTokenXYZ"));

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .Times(1)
      .WillOnce(Return("dc85740d-cb27-405b-9da3-e8be5c71ae5b"));

  const char postPayload2[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"AccessTokenXYZ\"},\"endpointId\":"
      "\"dc85740d-cb27-405b-9da3-e8be5c71ae5b-567\"},\"payload\":{\"change\":{"
      "\"cause\":{\"type\":\"APP_INTERACTION\"},\"properties\":[{\"namespace\":"
      "\"Alexa.EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":"
      "\"UNREACHABLE\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(postPayload2))).Times(1);

  EXPECT_CALL(*curlAdapter, reset).Times(2);

  EXPECT_CALL(*curlAdapter, perform).Times(2).WillRepeatedly(Return(true));

  supla_alexa_change_report_request2 *request =
      new supla_alexa_change_report_request2(supla_caller(ctDevice), 1, 2, 567,
                                             queue, pool, propertyGetter,
                                             &credentials);
  request->set_zulu_time("2019-02-01T12:09:33Z");
  request->set_message_id("29012dd1-33c7-6519-6e18-c4ee71d00487");
  request->set_delay_usec(0);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(AlexaTokenRefreshTest, refreshTokenNotExists) {
  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, expires_in).Times(1).WillOnce(Return(30));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .Times(1)
      .WillOnce(Return(false));

  EXPECT_CALL(credentials, get_set_time).Times(0);

  EXPECT_CALL(credentials, refresh_lock).Times(0);

  EXPECT_CALL(credentials, update).Times(0);

  EXPECT_CALL(credentials, remove).Times(0);

  supla_alexa_change_report_request2 *request =
      new supla_alexa_change_report_request2(supla_caller(ctDevice), 1, 2, 567,
                                             queue, pool, propertyGetter,
                                             &credentials);
  request->set_delay_usec(0);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(AlexaTokenRefreshTest, http401) { httpCodeTest(401); }

TEST_F(AlexaTokenRefreshTest, http403) { httpCodeTest(403); }

TEST_F(AlexaTokenRefreshTest, http404) { httpCodeTest(404); }

TEST_F(AlexaTokenRefreshTest, badCode) {
  badRequestTest("INVALID_ACCESS_TOKEN_EXCEPTION!", false);
}

TEST_F(AlexaTokenRefreshTest, invalidAccessTokenException) {
  badRequestTest("INVALID_ACCESS_TOKEN_EXCEPTION", true);
}

TEST_F(AlexaTokenRefreshTest, skillDisabledException) {
  badRequestTest("SKILL_DISABLED_EXCEPTION", true);
}

TEST_F(AlexaTokenRefreshTest, skillNotFoundException) {
  badRequestTest("SKILL_NOT_FOUND_EXCEPTION", true);
}

TEST_F(AlexaTokenRefreshTest, theTokenHasChangedInTheMeantime) {
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

  EXPECT_CALL(credentials, refresh_unlock).Times(1);

  EXPECT_CALL(credentials, get_region).WillOnce(Return(""));

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(200));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer AccessTokenXYZ")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://api.amazonalexa.com/v3/events")))
      .Times(1);

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("AccessTokenXYZ"));

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .Times(1)
      .WillOnce(Return("dc85740d-cb27-405b-9da3-e8be5c71ae5b"));

  const char postPayload2[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"AccessTokenXYZ\"},\"endpointId\":"
      "\"dc85740d-cb27-405b-9da3-e8be5c71ae5b-567\"},\"payload\":{\"change\":{"
      "\"cause\":{\"type\":\"APP_INTERACTION\"},\"properties\":[{\"namespace\":"
      "\"Alexa.EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":"
      "\"UNREACHABLE\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(postPayload2))).Times(1);

  EXPECT_CALL(*curlAdapter, reset).Times(1);

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillRepeatedly(Return(true));

  supla_alexa_change_report_request2 *request =
      new supla_alexa_change_report_request2(supla_caller(ctDevice), 1, 2, 567,
                                             queue, pool, propertyGetter,
                                             &credentials);
  request->set_zulu_time("2019-02-01T12:09:33Z");
  request->set_message_id("29012dd1-33c7-6519-6e18-c4ee71d00487");
  request->set_delay_usec(0);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

}  // namespace testing
