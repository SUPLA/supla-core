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

#include "AlexaDeleteRequestTest.h"

#include <memory>
#include <vector>

#include "amazon/alexa_delete_request.h"
#include "http/asynctask_http_thread_bucket.h"

namespace testing {

using std::string;

void AlexaDeleteRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("ACCESS-TOKEN"));

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .WillRepeatedly(Return("qwerty"));

  ON_CALL(*curlAdapter, get_response_code).WillByDefault(Return(200));
}

TEST_F(AlexaDeleteRequestTest, makeRequestWithSuccess) {
  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer ACCESS-TOKEN")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://api.amazonalexa.com/v3/events")))
      .Times(1);

  EXPECT_CALL(
      *curlAdapter,
      set_opt_post_fields(
          StrEq("{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
                "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
                "\"DeleteReport\",\"payloadVersion\":\"3\"},\"payload\":{"
                "\"endpoints\":[{\"endpointId\":\"qwerty-1\"},{\"endpointId\":"
                "\"qwerty-2\"},{\"endpointId\":\"qwerty-3\"},{\"endpointId\":"
                "\"qwerty-4\"},{\"endpointId\":\"qwerty-5\"},{\"endpointId\":"
                "\"qwerty:scene-1\"},{\"endpointId\":\"qwerty:scene-2\"},{"
                "\"endpointId\":\"qwerty:scene-3\"},{\"endpointId\":\"qwerty:"
                "scene-4\"},{\"endpointId\":\"qwerty:scene-5\"}]}}}")))
      .Times(1);

  std::vector<int> ids{1, 2, 3, 3, 4, 5, 4};

  supla_alexa_delete_request *request =
      new supla_alexa_delete_request(2, queue, pool, &credentials);
  request->add(ids, false);
  request->add(ids, false);
  request->add(ids, true);
  request->add(ids, true);
  request->set_delay_usec(1);
  request->set_message_id("29012dd1-33c7-6519-6e18-c4ee71d00487");
  request->set_zulu_time("2019-02-01T12:09:33Z");
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

}  // namespace testing
