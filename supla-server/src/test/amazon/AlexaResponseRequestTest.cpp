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

#include "AlexaResponseRequestTest.h"

#include <memory>

#include "amazon/alexa_response_request.h"
#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "device/extended_value/channel_thermostat_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_hp_thermostat_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "http/asynctask_http_thread_bucket.h"
#include "jsonconfig/channel/hvac_config.h"

namespace testing {

using std::string;

void AlexaResponseRequestTest::SetUp(void) {
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

void AlexaResponseRequestTest::makeTest(int func, bool online,
                                        supla_channel_value *value,
                                        const char *expectedPayload,
                                        const string &correlation_token) {
  EXPECT_CALL(*curlAdapter,
              append_header(Eq(0), StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(Eq(0), StrEq("Authorization: Bearer ACCESS-TOKEN")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(15), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([func, online, value](
                    int user_id, int device_id, int channel_id,
                    supla_channel_fragment *_fragment,
                    supla_channel_availability_status *_status) {
        *_fragment =
            supla_channel_fragment(device_id, channel_id, 0, 0, func, 0, false);
        _status->set_offline(!online);

        return value;
      });

  EXPECT_CALL(credentials, get_region).WillRepeatedly(Return("eu"));

  EXPECT_CALL(
      *curlAdapter,
      set_opt_url(Eq(0), StrEq("https://api.eu.amazonalexa.com/v3/events")))
      .Times(1);

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  supla_alexa_response_request *request = new supla_alexa_response_request(
      supla_caller(), 1, 2, 15, queue, pool, propertyGetter, &credentials,
      correlation_token);
  request->set_delay_usec(1);
  request->set_message_id("29012dd1-33c7-6519-6e18-c4ee71d00487");
  request->set_zulu_time("2019-02-01T12:09:33Z");
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

void AlexaResponseRequestTest::makeTest(int func, bool online,
                                        supla_channel_value *value,
                                        const char *expectedPayload) {
  makeTest(func, online, value, expectedPayload, "correlationToken");
}

TEST_F(AlexaResponseRequestTest, power_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"Response\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{}}}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, true,
           new supla_channel_onoff_value(true), expectedPayload);
}

TEST_F(AlexaResponseRequestTest, power_Unreachable) {
  const char expectedPayload[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  makeTest(SUPLA_CHANNELFNC_POWERSWITCH, false,
           new supla_channel_onoff_value(true), expectedPayload);
}

TEST_F(AlexaResponseRequestTest, brightness_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":10,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-15\"},\"payload\":{}}}";

  TRGBW_Value rgbw = {};
  rgbw.brightness = 10;

  makeTest(SUPLA_CHANNELFNC_DIMMER, true, new supla_channel_rgbw_value(&rgbw),
           expectedPayload);
}

TEST_F(AlexaResponseRequestTest, brightness_Unreachable) {
  const char expectedPayload[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  makeTest(SUPLA_CHANNELFNC_DIMMER, false, new supla_channel_rgbw_value(),
           expectedPayload);
}

TEST_F(AlexaResponseRequestTest, color_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ColorController\",\"name\":\"color\",\"value\":{\"hue\":348,"
      "\"saturation\":0.33333333333333331,\"brightness\":0.1},\"timeOfSample\":"
      "\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":50},{"
      "\"namespace\":\"Alexa.BrightnessController\",\"name\":\"brightness\","
      "\"value\":10,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"Response\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{}}}";

  TRGBW_Value rgbw = {};
  rgbw.R = 0xFF;
  rgbw.G = 0xAA;
  rgbw.B = 0xBB;
  rgbw.colorBrightness = 10;

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload);
}

TEST_F(AlexaResponseRequestTest, color_Unreachable) {
  const char expectedPayload[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, false, new supla_channel_rgbw_value(),
           expectedPayload);
}

TEST_F(AlexaResponseRequestTest, brightnessAndColor_ColorReachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ColorController\",\"name\":\"color\",\"value\":{\"hue\":120,"
      "\"saturation\":1,\"brightness\":1},\"timeOfSample\":\"2019-02-01T12:09:"
      "33Z\",\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":100,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-15-1\"},\"payload\":{}}}";

  TRGBW_Value rgbw = {};
  rgbw.colorBrightness = 100;
  rgbw.G = 0xFF;

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload,
           "correlationToken::SUB=1");
}

TEST_F(AlexaResponseRequestTest, brightnessAndColor_BrightnessReachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":100,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-15-2\"},\"payload\":{}}}";

  TRGBW_Value rgbw = {};
  rgbw.brightness = 100;

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload,
           "correlationToken::SUB=2");
}

TEST_F(AlexaResponseRequestTest, brightnessAndColor_ColorUnreachable) {
  const char expectedPayload1[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15-1\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, false,
           new supla_channel_rgbw_value(), expectedPayload1,
           "correlationToken::SUB=1");
}

TEST_F(AlexaResponseRequestTest, brightnessAndColor_BrightnessUnreachable) {
  const char expectedPayload[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15-2\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, false,
           new supla_channel_rgbw_value(), expectedPayload,
           "correlationToken::SUB=2");
}

TEST_F(AlexaResponseRequestTest, percentage_Reachable) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "PercentageController\",\"name\":\"percentage\",\"value\":60,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"Response\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{}}}";

  TDSC_RollerShutterValue rs_val = {};
  rs_val.position = 60;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
           new supla_channel_rs_value(&rs_val), expectedPayload);
}

TEST_F(AlexaResponseRequestTest, percentage_Unreachable) {
  const char expectedPayload[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  TDSC_RollerShutterValue rs_val = {};

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, false,
           new supla_channel_rs_value(&rs_val), expectedPayload);
}

void AlexaResponseRequestTest::hvacThermostatTest(
    int func, bool online, supla_channel_value *hvacValue,
    supla_channel_value *tempHumValue,
    supla_channel_extended_value *extendedValue, const char *expectedPayload,
    const string &correlation_token) {
  EXPECT_CALL(*curlAdapter,
              append_header(Eq(0), StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter,
              append_header(Eq(0), StrEq("Authorization: Bearer ACCESS-TOKEN")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(15), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([func, online, hvacValue](
                    int user_id, int device_id, int channel_id,
                    supla_channel_fragment *_fragment,
                    supla_channel_availability_status *_status) {
        *_fragment =
            supla_channel_fragment(device_id, channel_id, 0, 0, func, 0, false);
        _status->set_offline(!online);

        return hvacValue;
      });

  if (online) {
    if (tempHumValue) {
      EXPECT_CALL(*propertyGetter,
                  _get_detached_json_config(Eq(1), Eq(2), Eq(15)))
          .WillOnce([](int user_id, int device_id, int channel_id) {
            TChannelConfig_HVAC native_config = {};
            native_config.MainThermometerChannelNo = 3;
            hvac_config *cfg = new hvac_config();
            cfg->set_config(&native_config, 1);
            return cfg;
          });

      EXPECT_CALL(*propertyGetter, _get_channel_id(Eq(1), Eq(2), Eq(3)))
          .WillOnce(Return(789));

      EXPECT_CALL(*propertyGetter,
                  _get_value(Eq(1), Eq(2), Eq(789), IsNull(), IsNull()))
          .WillOnce(Return(tempHumValue));
    } else {
      EXPECT_CALL(*propertyGetter, _get_extended_value(Eq(1), Eq(2), Eq(15)))
          .WillOnce(Return(extendedValue));
    }
  }

  EXPECT_CALL(credentials, get_region).WillRepeatedly(Return("eu"));

  EXPECT_CALL(
      *curlAdapter,
      set_opt_url(Eq(0), StrEq("https://api.eu.amazonalexa.com/v3/events")))
      .Times(1);

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  supla_alexa_response_request *request = new supla_alexa_response_request(
      supla_caller(), 1, 2, 15, queue, pool, propertyGetter, &credentials,
      correlation_token);
  request->set_delay_usec(1);
  request->set_message_id("29012dd1-33c7-6519-6e18-c4ee71d00487");
  request->set_zulu_time("2019-02-01T12:09:33Z");
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(AlexaResponseRequestTest, thermostat) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ThermostatController\",\"name\":\"thermostatMode\",\"value\":\"HEAT\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.ThermostatController\",\"name\":"
      "\"targetSetpoint\",\"value\":{\"value\":22.33,\"scale\":\"CELSIUS\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.TemperatureSensor\",\"name\":\"temperature\","
      "\"value\":{\"value\":22.3,\"scale\":\"CELSIUS\"},\"timeOfSample\":"
      "\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":50}]},\"event\":{"
      "\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\","
      "\"namespace\":\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken123\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-15\"},\"payload\":{}}}";

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_hvac_value hvac;
  hvac.set_mode(SUPLA_HVAC_MODE_HEAT);
  hvac.set_setpoint_temperature_heat(2233);
  hvac.get_raw_value(raw_value);

  hvacThermostatTest(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, true,
                     new supla_channel_hvac_value(raw_value),
                     new supla_channel_temphum_value(true, 22.30, 45), nullptr,
                     expectedPayload, "correlationToken123");
}

TEST_F(AlexaResponseRequestTest, thermostat_Unreachable) {
  const char expectedPayload[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  hvacThermostatTest(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, false, nullptr, nullptr,
                     nullptr, expectedPayload, "correlationToken");
}

TEST_F(AlexaResponseRequestTest, thermostat_HeatCool) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ThermostatController\",\"name\":\"thermostatMode\",\"value\":\"AUTO\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.ThermostatController\",\"name\":"
      "\"lowerSetpoint\",\"value\":{\"value\":22.33,\"scale\":\"CELSIUS\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.ThermostatController\",\"name\":"
      "\"upperSetpoint\",\"value\":{\"value\":25.55,\"scale\":\"CELSIUS\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.TemperatureSensor\",\"name\":\"temperature\","
      "\"value\":{\"value\":22.3,\"scale\":\"CELSIUS\"},\"timeOfSample\":"
      "\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":50}]},\"event\":{"
      "\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\","
      "\"namespace\":\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken123\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-15\"},\"payload\":{}}}";

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_hvac_value hvac;
  hvac.set_mode(SUPLA_HVAC_MODE_HEAT_COOL);
  hvac.set_setpoint_temperature_heat(2233);
  hvac.set_setpoint_temperature_cool(2555);
  hvac.get_raw_value(raw_value);

  hvacThermostatTest(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, true,
                     new supla_channel_hvac_value(raw_value),
                     new supla_channel_temphum_value(true, 22.30, 45), nullptr,
                     expectedPayload, "correlationToken123");
}

TEST_F(AlexaResponseRequestTest, thermostat_HeatPol) {
  const char expectedPayload[] =
      "{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ThermostatController\",\"name\":\"thermostatMode\",\"value\":\"HEAT\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.ThermostatController\",\"name\":"
      "\"targetSetpoint\",\"value\":{\"value\":45.67,\"scale\":\"CELSIUS\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.TemperatureSensor\",\"name\":\"temperature\","
      "\"value\":{\"value\":12.34,\"scale\":\"CELSIUS\"},\"timeOfSample\":"
      "\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":50}]},\"event\":{"
      "\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\","
      "\"namespace\":\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken123\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-15\"},\"payload\":{}}}";

  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;
  native_th_ev.Flags[4] = HP_STATUS_POWERON;
  srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->MeasuredTemperature = 1234;
  ((TThermostat_Value *)raw_value)->PresetTemperature = 4567;

  hvacThermostatTest(SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS, true,
                     new supla_channel_hp_thermostat_value(raw_value), nullptr,
                     new supla_channel_thermostat_extended_value(&native_ev),
                     expectedPayload, "correlationToken123");
}

TEST_F(AlexaResponseRequestTest, thermostat_HeatPol_Disconnected) {
  const char expectedPayload[] =
      "{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken123\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";

  hvacThermostatTest(SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS, false,
                     nullptr, nullptr, nullptr, expectedPayload,
                     "correlationToken123");
}

}  // namespace testing
