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

#include "AlexaChangeReportRequestTest.h"

#include "amazon/alexa_change_report_request.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "http/asynctask_http_thread_bucket.h"

namespace testing {

using std::string;

void AlexaChangeReportRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();
  propertyGetter = new ChannelPropertyGetterMock();

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("ACCESS-TOKEN"));

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .WillRepeatedly(Return("qwerty"));

  ON_CALL(*curlAdapter, get_response_code).WillByDefault(Return(200));
}

void AlexaChangeReportRequestTest::makeTest(int func, bool online,
                                            supla_channel_value *value,
                                            const char *expectedPayload,
                                            const supla_caller &caller,
                                            const std::string &region) {
  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer ACCESS-TOKEN")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(3), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([func, online, value](
                    int user_id, int device_id, int channel_id,
                    supla_channel_fragment *_fragment, bool *_Reachable) {
        *_fragment =
            supla_channel_fragment(device_id, channel_id, 0, func, 0, false);
        *_Reachable = online;

        return value;
      });

  EXPECT_CALL(credentials, get_region).WillRepeatedly(Return(region));

  string expected_url = "https://api";
  if (region.size()) {
    expected_url.append(".");
    expected_url.append(region);
  }
  expected_url.append(".amazonalexa.com/v3/events");

  EXPECT_CALL(*curlAdapter, set_opt_url(StrEq(expected_url))).Times(1);

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(expectedPayload)))
      .Times(1);

  supla_alexa_change_report_request *request =
      new supla_alexa_change_report_request(caller, 1, 2, 3, queue, pool,
                                            propertyGetter, &credentials);
  request->set_delay_usec(1);
  request->set_message_id("29012dd1-33c7-6519-6e18-c4ee71d00487");
  request->set_zulu_time("2019-02-01T12:09:33Z");
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

void AlexaChangeReportRequestTest::makeTest(int func, bool online,
                                            supla_channel_value *value,
                                            const char *expectedPayload1,
                                            const char *expectedPayload2) {
  EXPECT_CALL(*curlAdapter, reset).Times(2);

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(2)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(StrEq("Authorization: Bearer ACCESS-TOKEN")))
      .Times(2)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(2).WillRepeatedly(Return(true));

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(3), NotNull(), NotNull()))
      .Times(1)
      .WillRepeatedly([func, online, value](
                          int user_id, int device_id, int channel_id,
                          supla_channel_fragment *_fragment, bool *_Reachable) {
        *_fragment = supla_channel_fragment(
            device_id, channel_id, 0, func, 0, false);
        *_Reachable = online;

        return value;
      });

  EXPECT_CALL(credentials, get_region).WillRepeatedly(Return(""));

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://api.amazonalexa.com/v3/events")))
      .Times(2);

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(expectedPayload1)))
      .Times(1);

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(expectedPayload2)))
      .Times(1);

  supla_alexa_change_report_request *request =
      new supla_alexa_change_report_request(supla_caller(ctAmazonAlexa), 1, 2,
                                            3, queue, pool, propertyGetter,
                                            &credentials);
  request->set_delay_usec(1);
  request->set_message_id("29012dd1-33c7-6519-6e18-c4ee71d00487");
  request->set_zulu_time("2019-02-01T12:09:33Z");
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(AlexaChangeReportRequestTest, power_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa.PowerController\","
      "\"name\":\"powerState\",\"value\":\"ON\",\"timeOfSample\":\"2019-02-"
      "01T12:09:33Z\",\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\","
      "\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":"
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-3\"}"
      ",\"payload\":{\"change\":{\"cause\":{\"type\":\"APP_INTERACTION\"},"
      "\"properties\":[{\"namespace\":\"Alexa.PowerController\",\"name\":"
      "\"powerState\",\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:"
      "33Z\",\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, true,
           new supla_channel_onoff_value(true), expectedPayload,
           supla_caller(ctClient), "");
}

TEST_F(AlexaChangeReportRequestTest, power_Unreachable) {
  const char expectedPayload[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"APP_"
      "INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.EndpointHealth\","
      "\"name\":\"connectivity\",\"value\":{\"value\":\"UNREACHABLE\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  makeTest(SUPLA_CHANNELFNC_POWERSWITCH, false,
           new supla_channel_onoff_value(true), expectedPayload,
           supla_caller(ctClient), "");
}

TEST_F(AlexaChangeReportRequestTest, contanct_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa.ContactSensor\","
      "\"name\":\"detectionState\",\"value\":\"DETECTED\",\"timeOfSample\":"
      "\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":50},{"
      "\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-3\"},\"payload\":{\"change\":{"
      "\"cause\":{\"type\":\"PHYSICAL_INTERACTION\"},\"properties\":[{"
      "\"namespace\":\"Alexa.ContactSensor\",\"name\":\"detectionState\","
      "\"value\":\"DETECTED\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR, true,
           new supla_channel_binary_sensor_value(false), expectedPayload,
           supla_caller(ctDevice), "");
}

TEST_F(AlexaChangeReportRequestTest, contact_Unreachable) {
  const char expectedPayload[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"PHYSICAL_"
      "INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.EndpointHealth\","
      "\"name\":\"connectivity\",\"value\":{\"value\":\"UNREACHABLE\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR, false,
           new supla_channel_binary_sensor_value(true), expectedPayload,
           supla_caller(ctDevice), "en");
}

TEST_F(AlexaChangeReportRequestTest, brightness_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":10,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\","
      "\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":"
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-3\"}"
      ",\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_INTERACTION\"},"
      "\"properties\":[{\"namespace\":\"Alexa.BrightnessController\",\"name\":"
      "\"brightness\",\"value\":10,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  TRGBW_Value rgbw = {};
  rgbw.brightness = 10;

  makeTest(SUPLA_CHANNELFNC_DIMMER, true, new supla_channel_rgbw_value(&rgbw),
           expectedPayload, supla_caller(ctAmazonAlexa), "");
}

TEST_F(AlexaChangeReportRequestTest, brightness_Unreachable) {
  const char expectedPayload[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_"
      "INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.EndpointHealth\","
      "\"name\":\"connectivity\",\"value\":{\"value\":\"UNREACHABLE\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  makeTest(SUPLA_CHANNELFNC_DIMMER, false, new supla_channel_rgbw_value(),
           expectedPayload, supla_caller(ctAmazonAlexa), "");
}

TEST_F(AlexaChangeReportRequestTest, color_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa.ColorController\","
      "\"name\":\"color\",\"value\":{\"hue\":348,\"saturation\":0."
      "33333333333333331,\"brightness\":0.1},\"timeOfSample\":\"2019-02-01T12:"
      "09:33Z\",\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":10,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\","
      "\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":"
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-3\"}"
      ",\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_INTERACTION\"},"
      "\"properties\":[{\"namespace\":\"Alexa.ColorController\",\"name\":"
      "\"color\",\"value\":{\"hue\":348,\"saturation\":0.33333333333333331,"
      "\"brightness\":0.1},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":10,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  TRGBW_Value rgbw = {};
  rgbw.R = 0xFF;
  rgbw.G = 0xAA;
  rgbw.B = 0xBB;
  rgbw.colorBrightness = 10;

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload,
           supla_caller(ctAmazonAlexa), "");
}

TEST_F(AlexaChangeReportRequestTest, color_Unreachable) {
  const char expectedPayload[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_"
      "INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.EndpointHealth\","
      "\"name\":\"connectivity\",\"value\":{\"value\":\"UNREACHABLE\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, false, new supla_channel_rgbw_value(),
           expectedPayload, supla_caller(ctAmazonAlexa), "");
}

TEST_F(AlexaChangeReportRequestTest, percentage_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa.RangeController\","
      "\"instance\":\"Blind.Lift\",\"name\":\"rangeValue\",\"value\":60,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PercentageController\",\"name\":"
      "\"percentage\",\"value\":60,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\","
      "\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":"
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-3\"}"
      ",\"payload\":{\"change\":{\"cause\":{\"type\":\"PHYSICAL_INTERACTION\"},"
      "\"properties\":[{\"namespace\":\"Alexa.RangeController\",\"instance\":"
      "\"Blind.Lift\",\"name\":\"rangeValue\",\"value\":60,\"timeOfSample\":"
      "\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":50},{"
      "\"namespace\":\"Alexa.PercentageController\",\"name\":\"percentage\","
      "\"value\":60,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  TDSC_RollerShutterValue rs_val = {};
  rs_val.position = 60;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
           new supla_channel_rs_value(&rs_val), expectedPayload,
           supla_caller(ctDevice), "");
}

TEST_F(AlexaChangeReportRequestTest, percentage_Unreachable) {
  const char expectedPayload[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"PHYSICAL_"
      "INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.EndpointHealth\","
      "\"name\":\"connectivity\",\"value\":{\"value\":\"UNREACHABLE\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  TDSC_RollerShutterValue rs_val = {};

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, false,
           new supla_channel_rs_value(&rs_val), expectedPayload,
           supla_caller(ctDevice), "");
}

TEST_F(AlexaChangeReportRequestTest, brightnessAndColor_Reachable) {
  const char expectedPayload1[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ColorController\",\"name\":\"color\",\"value\":{\"hue\":120,"
      "\"saturation\":1,\"brightness\":1},\"timeOfSample\":\"2019-02-01T12:09:"
      "33Z\",\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":100,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\","
      "\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":"
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-3-"
      "1\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_INTERACTION\"}"
      ",\"properties\":[{\"namespace\":\"Alexa.ColorController\",\"name\":"
      "\"color\",\"value\":{\"hue\":120,\"saturation\":1,\"brightness\":1},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.BrightnessController\",\"name\":"
      "\"brightness\",\"value\":100,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  const char expectedPayload2[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":100,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ChangeReport\","
      "\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{\"type\":"
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-3-"
      "2\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_INTERACTION\"}"
      ",\"properties\":[{\"namespace\":\"Alexa.BrightnessController\",\"name\":"
      "\"brightness\",\"value\":100,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  TRGBW_Value rgbw = {};
  rgbw.brightness = 100;
  rgbw.colorBrightness = 100;

  rgbw.G = 0xFF;

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload1,
           expectedPayload2);
}

TEST_F(AlexaChangeReportRequestTest, brightnessAndColor_Unreachable) {
  const char expectedPayload1[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3-1\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_"
      "INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.EndpointHealth\","
      "\"name\":\"connectivity\",\"value\":{\"value\":\"UNREACHABLE\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  const char expectedPayload2[] =
      "{\"context\":{},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-"
      "6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":"
      "\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3-2\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_"
      "INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa.EndpointHealth\","
      "\"name\":\"connectivity\",\"value\":{\"value\":\"UNREACHABLE\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, false,
           new supla_channel_rgbw_value(), expectedPayload1, expectedPayload2);
}

}  // namespace testing
