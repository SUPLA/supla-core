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

#include "GoogleHomeStateReportRequestTest.h"

#include <memory>

#include "device/extended_value/channel_hp_thermostat_ev_decorator.h"
#include "device/extended_value/channel_thermostat_extended_value.h"
#include "device/value/channel_fb_value.h"
#include "device/value/channel_hp_thermostat_value.h"
#include "device/value/channel_hvac_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "google/google_home_state_report_request.h"
#include "http/asynctask_http_thread_bucket.h"
#include "jsonconfig/channel/hvac_config.h"

namespace testing {

using std::string;

void GoogleHomeStateReportRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();
  propertyGetter = new ChannelPropertyGetterMock();

  tokenProviderCurlAdapter = new CurlAdapterMock();
  tokenProvider = new AccessTokenProviderMock(tokenProviderCurlAdapter);

  ON_CALL(*tokenProviderCurlAdapter, perform).WillByDefault(Return(true));
  ON_CALL(*tokenProviderCurlAdapter, get_response_code)
      .WillByDefault(Return(200));

  ON_CALL(*tokenProviderCurlAdapter, set_opt_write_data)
      .WillByDefault([this](int instance_id, string *request_result) {
        *request_result =
            "{\"homegraph\":{\"0\":{\"token\":\"directTokenXyz\",\"expires_"
            "in\":3600}}}";
      });

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_user_long_unique_id)
      .WillRepeatedly(Return("zxcvbnm"));

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .WillRepeatedly(Return("qwerty"));

  EXPECT_CALL(*curlAdapter,
              append_header(Eq(0), StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  ON_CALL(*curlAdapter, get_response_code).WillByDefault(Return(200));
}

void GoogleHomeStateReportRequestTest::TearDown(void) {
  if (tokenProvider) {
    delete tokenProvider;
    tokenProvider = nullptr;
  }

  AsyncTaskTest::TearDown();
}

void GoogleHomeStateReportRequestTest::expectToken(bool direct) {
  if (direct) {
    EXPECT_CALL(*curlAdapter,
                set_opt_url(Eq(0), StrEq("https://homegraph.googleapis.com/v1/"
                                         "devices:reportStateAndNotification")))
        .Times(1);

    EXPECT_CALL(
        *curlAdapter,
        append_header(Eq(0), StrEq("Authorization: Bearer directTokenXyz")))
        .Times(1)
        .WillOnce(Return(true));
  } else {
    EXPECT_CALL(credentials, get_access_token)
        .WillRepeatedly(Return("MyAccessTokenXYZ"));

    EXPECT_CALL(*curlAdapter,
                set_opt_url(Eq(0), StrEq("https://"
                                         "odokilkqoesh73zfznmiupey4a0uugaz."
                                         "lambda-url.eu-west-1.on.aws/")))
        .Times(1);

    EXPECT_CALL(
        *curlAdapter,
        append_header(Eq(0), StrEq("Authorization: Bearer MyAccessTokenXYZ")))
        .Times(1)
        .WillOnce(Return(true));
  }
}

void GoogleHomeStateReportRequestTest::makeTest(
    int func, bool online, supla_abstract_channel_value *value,
    const char *expectedPayload, const string &request_id, bool direct) {
  expectToken(direct);

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(10), NotNull(), NotNull()))
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

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  if (direct) {
    tokenProvider->refresh();
  }

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, tokenProvider, request_id);
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

void GoogleHomeStateReportRequestTest::makeTest(
    int func, bool online, supla_abstract_channel_value *value,
    const char *expectedPayload) {
  makeTest(func, online, value, expectedPayload,
           "e2de5bc6-65a8-48e5-b919-8a48e86ad64a", false);
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
           new supla_channel_onoff_value(true), expectedPayload, "REQID",
           false);
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
           expectedPayload, "REQID", false);
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
           new supla_channel_rgbw_value(&rgbw), expectedPayload, "REQID",
           false);
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
           new supla_channel_rgbw_value(&rgbw), expectedPayload, "REQID",
           false);
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
           new supla_channel_rs_value(&rs), expectedPayload, "REQID", false);
}

TEST_F(GoogleHomeStateReportRequestTest, rollershutter_80) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"openPercent\":"
      "20}}}}}";

  TDSC_RollerShutterValue rs = {};
  rs.position = 80;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
           new supla_channel_rs_value(&rs), expectedPayload, "REQID", false);
}

TEST_F(GoogleHomeStateReportRequestTest, facadeblind_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\",\"agentUserId\":"
      "\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{"
      "\"online\":false,\"openPercent\":100,\"rotationPercent\":100}}}}}";

  TDSC_FacadeBlindValue fb = {};
  fb.position = 56;
  fb.tilt = 14;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, false,
           new supla_channel_fb_value(&fb), expectedPayload);
}

TEST_F(GoogleHomeStateReportRequestTest, facadeblind_Connected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"openPercent\":"
      "44,\"rotationPercent\":86}}}}}";

  TDSC_FacadeBlindValue fb = {};
  fb.position = 56;
  fb.tilt = 14;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, true,
           new supla_channel_fb_value(&fb), expectedPayload, "REQID", false);
}

void GoogleHomeStateReportRequestTest::makeHvacThermostatTest(
    int func, bool online, supla_abstract_channel_value *hvacValue,
    supla_abstract_channel_value *tempHumValue,
    supla_channel_extended_value *extendedValue, const char *expectedPayload,
    const string &request_id, bool direct) {
  expectToken(direct);

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(10), NotNull(), NotNull()))
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
                  _get_detached_json_config(Eq(1), Eq(2), Eq(10)))
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
      EXPECT_CALL(*propertyGetter, _get_extended_value(Eq(1), Eq(2), Eq(10)))
          .WillOnce(Return(extendedValue));
    }
  }

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, tokenProvider, request_id);
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(GoogleHomeStateReportRequestTest, thermostat) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"thermostatMode\":\"heat\",\"thermostatTemperatureSetpoint\":22.33,"
      "\"thermostatTemperatureAmbient\":22.3,\"thermostatHumidityAmbient\":45}}"
      "}}}";

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_hvac_value hvac;
  hvac.set_mode(SUPLA_HVAC_MODE_HEAT);
  hvac.set_setpoint_temperature_heat(2233);
  hvac.get_raw_value(raw_value);

  makeHvacThermostatTest(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, true,
                         new supla_channel_hvac_value(raw_value),
                         new supla_channel_temphum_value(true, 22.30, 45),
                         nullptr, expectedPayload, "REQID", false);
}

TEST_F(GoogleHomeStateReportRequestTest, thermostat_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":false}}}}}";

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_hvac_value hvac;
  hvac.set_mode(SUPLA_HVAC_MODE_HEAT);
  hvac.set_setpoint_temperature_heat(2233);
  hvac.get_raw_value(raw_value);

  makeHvacThermostatTest(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, false, nullptr,
                         nullptr, nullptr, expectedPayload, "REQID", false);
}

TEST_F(GoogleHomeStateReportRequestTest, thermostatHeatCool) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"thermostatMode\":\"heatcool\",\"thermostatTemperatureSetpointHigh\":"
      "25.55,\"thermostatTemperatureSetpointLow\":22.33,"
      "\"thermostatTemperatureAmbient\":22.3,\"thermostatHumidityAmbient\":45}}"
      "}}}";

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  supla_channel_hvac_value hvac;
  hvac.set_mode(SUPLA_HVAC_MODE_HEAT_COOL);
  hvac.set_setpoint_temperature_heat(2233);
  hvac.set_setpoint_temperature_cool(2555);
  hvac.get_raw_value(raw_value);

  makeHvacThermostatTest(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, true,
                         new supla_channel_hvac_value(raw_value),
                         new supla_channel_temphum_value(true, 22.30, 45),
                         nullptr, expectedPayload, "REQID", false);
}

TEST_F(GoogleHomeStateReportRequestTest, thermostatHeatPol) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"thermostatMode\":\"heat\",\"thermostatTemperatureSetpoint\":45.67,"
      "\"thermostatTemperatureAmbient\":12.34}}}}}";

  TSuplaChannelExtendedValue native_ev = {};
  TThermostat_ExtendedValue native_th_ev = {};
  native_th_ev.Fields = THERMOSTAT_FIELD_Flags;
  native_th_ev.Flags[4] = HP_STATUS_POWERON;
  srpc_evtool_v1_thermostatextended2extended(&native_th_ev, &native_ev);

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TThermostat_Value *)raw_value)->MeasuredTemperature = 1234;
  ((TThermostat_Value *)raw_value)->PresetTemperature = 4567;

  makeHvacThermostatTest(
      SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS, true,
      new supla_channel_hp_thermostat_value(raw_value), nullptr,
      new supla_channel_thermostat_extended_value(&native_ev), expectedPayload,
      "REQID", false);
}

TEST_F(GoogleHomeStateReportRequestTest, thermostatHeatPol_Disconnected) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":false}}}}}";

  makeHvacThermostatTest(SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS, false,
                         nullptr, nullptr, nullptr, expectedPayload, "REQID",
                         false);
}

TEST_F(GoogleHomeStateReportRequestTest, x403) {
  expectToken(false);

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(10), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id,
                   supla_channel_fragment *_fragment,
                   supla_channel_availability_status *_status) {
        *_fragment =
            supla_channel_fragment(device_id, channel_id, 0, 0,
                                   SUPLA_CHANNELFNC_LIGHTSWITCH, 0, false);
        _status->set_offline(true);

        return nullptr;
      });

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(403));

  EXPECT_CALL(credentials, exclude_channel).Times(1);

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, tokenProvider, "");
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(GoogleHomeStateReportRequestTest, x404) {
  expectToken(false);

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(10), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id,
                   supla_channel_fragment *_fragment,
                   supla_channel_availability_status *_status) {
        *_fragment =
            supla_channel_fragment(device_id, channel_id, 0, 0,
                                   SUPLA_CHANNELFNC_LIGHTSWITCH, 0, false);
        _status->set_offline(true);

        return nullptr;
      });

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(404));

  EXPECT_CALL(credentials, exclude_channel).Times(1);

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          supla_caller(ctDevice), 1, 2, 10, queue, pool, propertyGetter,
          &credentials, tokenProvider, "");
  request->set_delay_usec(1);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::FAILURE, 10000);
}

TEST_F(GoogleHomeStateReportRequestTest, onOff_Connected_Direct) {
  const char expectedPayload[] =
      "{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\",\"payload\":{"
      "\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,\"on\":true}}}}"
      "}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, true,
           new supla_channel_onoff_value(true), expectedPayload, "REQID", true);
}

}  // namespace testing
