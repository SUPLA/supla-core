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

#include <google/google_home_state_report_request.h>
#include "google/GoogleHomeStateReportRequestTest.h"

#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
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
                                                const char *expectedPayload,
                                                const string &request_id) {
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

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, request_id);
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

void GoogleHomeStateReportRequestTest::makeTest(int func, bool online,
                                                supla_channel_value *value,
                                                const char *expectedPayload) {
  makeTest(func, online, value, expectedPayload,
           "e2de5bc6-65a8-48e5-b919-8a48e86ad64a");
}

TEST_F(GoogleHomeStateReportRequestTest, onOff_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{"
      "\"online\":false,\"on\":false}}}}}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, false,
           new supla_channel_onoff_value(false), expectedPayload);
}

TEST_F(GoogleHomeStateReportRequestTest, onOff_Connected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"on\":true}}}}"
      "}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, true,
           new supla_channel_onoff_value(true), expectedPayload, "REQID");
}

TEST_F(GoogleHomeStateReportRequestTest, brightness_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{"
      "\"online\":false,\"on\":false,\"brightness\":0}}}}}";

  makeTest(SUPLA_CHANNELFNC_DIMMER, false, new supla_channel_rgbw_value(),
           expectedPayload);
}

TEST_F(GoogleHomeStateReportRequestTest, brightness_Connected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"on\":true,"
      "\"brightness\":55}}}}}";

  TRGBW_Value rgbw = {};
  rgbw.brightness = 55;

  makeTest(SUPLA_CHANNELFNC_DIMMER, true, new supla_channel_rgbw_value(&rgbw),
           expectedPayload, "REQID");
}

TEST_F(GoogleHomeStateReportRequestTest, colorAndBrightness_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{\"qwerty-10-1\":{"
      "\"online\":false,\"color\":{\"spectrumRGB\":0},\"on\":false,"
      "\"brightness\":0},\"qwerty-10-2\":{\"online\":false,\"on\":false,"
      "\"brightness\":0}}}}}";

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, false,
           new supla_channel_rgbw_value(), expectedPayload);
}

TEST_F(GoogleHomeStateReportRequestTest, colorAndBrightness_Connected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10-1\":{\"online\":true,\"color\":{"
      "\"spectrumRGB\":11189196},\"on\":true,\"brightness\":90},\"qwerty-10-"
      "2\":{\"online\":true,\"on\":true,\"brightness\":80}}}}}";

  TRGBW_Value rgbw = {};
  rgbw.brightness = 80;
  rgbw.colorBrightness = 90;
  rgbw.R = 0xAA;
  rgbw.G = 0xBB;
  rgbw.B = 0xCC;

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload, "REQID");
}

TEST_F(GoogleHomeStateReportRequestTest, color_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{"
      "\"online\":false,\"color\":{\"spectrumRGB\":16711867},\"on\":false,"
      "\"brightness\":0}}}}}";

  TRGBW_Value rgbw = {};
  rgbw.R = 0xFF;
  rgbw.G = 0x00;
  rgbw.B = 0xBB;

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, false,
           new supla_channel_rgbw_value(&rgbw), expectedPayload);
}

TEST_F(GoogleHomeStateReportRequestTest, color_Connected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"color\":{"
      "\"spectrumRGB\":4486946},\"on\":true,\"brightness\":55}}}}}";

  TRGBW_Value rgbw = {};
  rgbw.colorBrightness = 55;
  rgbw.R = 0x44;
  rgbw.G = 0x77;
  rgbw.B = 0x22;

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload, "REQID");
}

TEST_F(GoogleHomeStateReportRequestTest, rollershutter_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{"
      "\"online\":false,\"openPercent\":100}}}}}";

  TDSC_RollerShutterValue rs = {};
  rs.position = 55;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, false,
           new supla_channel_rs_value(&rs), expectedPayload);
}

TEST_F(GoogleHomeStateReportRequestTest, rollershutter_Connected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"openPercent\":"
      "45}}}}}";

  TDSC_RollerShutterValue rs = {};
  rs.position = 55;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
           new supla_channel_rs_value(&rs), expectedPayload, "REQID");
}

TEST_F(GoogleHomeStateReportRequestTest, rollershutter_80) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"openPercent\":"
      "20}}}}}";

  TDSC_RollerShutterValue rs = {};
  rs.position = 80;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
           new supla_channel_rs_value(&rs), expectedPayload, "REQID");
}

TEST_F(GoogleHomeStateReportRequestTest, x403) {
  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(10), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id, int *_func,
                   bool *_connected) {
        *_func = SUPLA_CHANNELFNC_LIGHTSWITCH;
        *_connected = false;

        return nullptr;
      });

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(403));

  EXPECT_CALL(credentials, exclude_channel).Times(1);

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, "");
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(GoogleHomeStateReportRequestTest, x404) {
  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(10), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id, int *_func,
                   bool *_connected) {
        *_func = SUPLA_CHANNELFNC_LIGHTSWITCH;
        *_connected = false;

        return nullptr;
      });

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(404));

  EXPECT_CALL(credentials, exclude_channel).Times(1);

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, "");
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

}  // namespace testing
