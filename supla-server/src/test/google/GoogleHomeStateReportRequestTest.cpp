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

#include "google/GoogleHomeStateReportRequestTest.h"

#include "device/value/channel_onoff_value.h"
#include "google/google_home_state_report_request2.h"
#include "http/asynctask_http_thread_bucket.h"

namespace testing {

using std::string;

void GoogleHomeStateReportRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();
  propertyGetter = new ChannelPropertyGetterMock();

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("MyAccessTokenXYZ"));

  EXPECT_CALL(credentials, get_user_long_unique_id)
      .WillRepeatedly(Return("zxcvbnm"));

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .WillRepeatedly(Return("qwerty"));

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

  ON_CALL(*curlAdapter, get_response_code).WillByDefault(Return(200));
}

void GoogleHomeStateReportRequestTest::makeTest(int func, bool online,
                                                supla_channel_value *value,
                                                const char *expectedPayload) {
  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(10), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([func, online, value](int user_id, int device_id,
                                      int channel_id, int *_func,
                                      bool *_connected) {
        *_func = func;
        *_connected = online;

        return value;
      });

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(expectedPayload)))
      .Times(1);

  supla_google_home_state_report_request2 *request =
      new supla_google_home_state_report_request2(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, "e2de5bc6-65a8-48e5-b919-8a48e86ad64a");
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(GoogleHomeStateReportRequestTest, onOff_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{"
      "\"online\":false,\"on\":false}}}}}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, false,
           new supla_channel_onoff_value(false), expectedPayload);
}

}  // namespace testing
