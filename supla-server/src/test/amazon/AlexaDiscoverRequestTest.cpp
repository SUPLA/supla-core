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

#include "AlexaDiscoverRequestTest.h"

#include <memory>
#include <string>

#include "amazon/alexa_discover_request.h"
#include "doubles/amazon/AlexaCredentialsDaoMock.h"
#include "doubles/amazon/AlexaDiscoverPayloadObtainerMock.h"
#include "http/asynctask_http_thread_bucket.h"

namespace testing {

using std::string;

void AlexaDiscoverRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("ACCESS-TOKEN"));

  ON_CALL(*curlAdapter, get_response_code).WillByDefault(Return(200));
}

TEST_F(AlexaDiscoverRequestTest, noAccessToken) {
  EXPECT_CALL(credentials, is_access_token_exists)
      .Times(1)
      .WillOnce(Return(false));

  supla_alexa_discover_request *request = new supla_alexa_discover_request(
      0, queue, pool, &credentials,
      new supla_alexa_discover_payload_obtainer(
          new AlexaCredentialsDaoMock(nullptr)));

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(AlexaDiscoverRequestTest, performFailed) {
  EXPECT_CALL(credentials, is_access_token_exists)
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, reset).Times(1);
  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(false));

  AlexaCredentialsDaoMock *dao = new AlexaCredentialsDaoMock(nullptr);
  EXPECT_CALL(*dao, get_cloud_access_token)
      .Times(1)
      .WillOnce(Return("xyzabcd"));

  supla_alexa_discover_request *request = new supla_alexa_discover_request(
      0, queue, pool, &credentials,
      new supla_alexa_discover_payload_obtainer(dao));

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(AlexaDiscoverRequestTest, http400) {
  EXPECT_CALL(credentials, is_access_token_exists)
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, reset).Times(1);
  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*curlAdapter, get_response_code).Times(1).WillOnce(Return(400));

  AlexaCredentialsDaoMock *dao = new AlexaCredentialsDaoMock(nullptr);
  EXPECT_CALL(*dao, get_cloud_access_token)
      .Times(1)
      .WillOnce(Return("xyzabcd"));

  supla_alexa_discover_request *request = new supla_alexa_discover_request(
      0, queue, pool, &credentials,
      new supla_alexa_discover_payload_obtainer(dao));

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(AlexaDiscoverRequestTest, wrongFormat) {
  EXPECT_CALL(credentials, is_access_token_exists)
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, reset).Times(1);
  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*curlAdapter, get_response_code).Times(1).WillOnce(Return(200));

  EXPECT_CALL(*curlAdapter, set_opt_write_data(NotNull()))
      .Times(1)
      .WillOnce([](string *result) {
        *result =
            "{\"event\":{\"header\":{\"namespace\":\"Alexa.Discovery\","
            "\"name\":\"Discover.Respnse\",\"payloadVersion\":\"3\","
            "\"messageId\":\"4fdcc64b-206c-4f48-bb32-e0c13324efa5\"}}}";
      });

  AlexaCredentialsDaoMock *dao = new AlexaCredentialsDaoMock(nullptr);
  EXPECT_CALL(*dao, get_cloud_access_token)
      .Times(1)
      .WillOnce(Return("xyzabcd"));

  supla_alexa_discover_request *request = new supla_alexa_discover_request(
      0, queue, pool, &credentials,
      new supla_alexa_discover_payload_obtainer(dao));

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(AlexaDiscoverRequestTest, obtaintWithSuccess) {
  AlexaCredentialsDaoMock *dao = new AlexaCredentialsDaoMock(nullptr);
  AlexaDiscoverPayloadObtainerMock *obtainer =
      new AlexaDiscoverPayloadObtainerMock(dao);

  EXPECT_CALL(*obtainer, get_message_id)
      .Times(2)
      .WillRepeatedly(Return("0021d4c9-1052-c70e-a701-3f75fa4b7712"));

  EXPECT_CALL(credentials, is_access_token_exists)
      .Times(3)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("ACCESS-TOKEN"));

  EXPECT_CALL(*curlAdapter, reset).Times(2);
  EXPECT_CALL(*curlAdapter, perform).Times(2).WillRepeatedly(Return(true));
  EXPECT_CALL(*curlAdapter, get_response_code)
      .Times(3)
      .WillRepeatedly(Return(200));

  {
    InSequence s;

    EXPECT_CALL(*curlAdapter, set_opt_write_data(NotNull()))
        .Times(1)
        .WillOnce([](string *result) {
          *result =
              "{\"event\":{\"header\":{\"namespace\":\"Alexa.Discovery\","
              "\"name\":\"Discover.Response\",\"payloadVersion\":\"3\","
              "\"messageId\":\"4fdcc64b-206c-4f48-bb32-e0c13324efa5\"}}}";
        });

    EXPECT_CALL(*curlAdapter, set_opt_write_data(NotNull()))
        .Times(1)
        .WillOnce([](string *result) { *result = ""; });
  }

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(2)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer ACCESS-TOKEN")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(
      *curlAdapter,
      set_opt_post_fields(StrEq(
          "{\"directive\":{\"header\":{\"namespace\":\"Alexa.Discovery\","
          "\"name\":\"Discover\",\"payloadVersion\":\"3\",\"messageId\":"
          "\"0021d4c9-1052-c70e-a701-3f75fa4b7712\"},\"payload\":{\"scope\":{"
          "\"type\":\"BearerToken\",\"token\":\"xyzabcd\"}}}}")))
      .Times(1);

  EXPECT_CALL(*curlAdapter,
              set_opt_post_fields(StrEq(
                  "{\"event\":{\"header\":{\"namespace\":\"Alexa.Discovery\","
                  "\"name\":\"AddOrUpdateReport\",\"payloadVersion\":\"3\","
                  "\"messageId\":\"0021d4c9-1052-c70e-a701-3f75fa4b7712\"}}}")))
      .Times(1);

  EXPECT_CALL(
      *curlAdapter,
      set_opt_url(StrEq(
          "https://"
          "kune6om4mlleevu2kh4vt2ic5i0otrmw.lambda-url.eu-west-1.on.aws")))
      .Times(1);

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://api.amazonalexa.com/v3/events")))
      .Times(1);

  EXPECT_CALL(*dao, get_cloud_access_token)
      .Times(1)
      .WillOnce(Return("xyzabcd"));

  supla_alexa_discover_request *request =
      new supla_alexa_discover_request(0, queue, pool, &credentials, obtainer);

  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

}  // namespace testing
