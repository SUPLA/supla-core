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

#include "webhook/StateWebhookRequestTest.h"

#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "http/asynctask_http_thread_bucket.h"
#include "webhook/state_webhook_request2.h"

namespace testing {

void StateWebhookRequestTest::SetUp(void) {
  AsyncTaskTest::SetUp();
  curlAdapter = new CurlAdapterMock();
  propertyGetter = new ChannelPropertyGetterMock();

  EXPECT_CALL(credentials, is_access_token_exists).WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, is_refresh_token_exists)
      .WillRepeatedly(Return(true));

  EXPECT_CALL(credentials, get_user_short_unique_id)
      .WillRepeatedly(Return("dc85740d-cb27-405b-9da3-e8be5c71ae5b"));

  EXPECT_CALL(credentials, expires_in).WillRepeatedly(Return(1000));

  EXPECT_CALL(credentials, get_access_token)
      .WillRepeatedly(Return("MyAccessTokenXYZ"));

  EXPECT_CALL(credentials, get_url)
      .WillRepeatedly(Return("https://webhook.test.io/endpoint"));

  EXPECT_CALL(*pool, get_bucket)
      .WillOnce(Return(new supla_asynctask_http_thread_bucket(curlAdapter)));

  EXPECT_CALL(*curlAdapter, get_timestamp).WillRepeatedly(Return(1600097258));

  EXPECT_CALL(*curlAdapter,
              set_opt_url(StrEq("https://webhook.test.io/endpoint")))
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
}

void StateWebhookRequestTest::TearDown(void) { AsyncTaskTest::TearDown(); }

void StateWebhookRequestTest::makeTest(int func, bool online,
                                       supla_channel_value *value,
                                       const char *expectedPayload) {
  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(123), NotNull(), NotNull()))
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

  supla_state_webhook_request2 *request = new supla_state_webhook_request2(
      supla_caller(ctDevice), 1, 2, 123, ET_CHANNEL_VALUE_CHANGED, 0, queue,
      pool, propertyGetter, &credentials);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 1000);
}

void StateWebhookRequestTest::makeTest(int func, bool online,
                                       supla_channel_ic_measurement *icm,
                                       const char *expectedPayload) {
  EXPECT_CALL(*propertyGetter, _get_ic_measurement(Eq(1), Eq(2), Eq(123)))
      .Times(1)
      .WillOnce(Return(icm));

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(123), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([func, online](int user_id, int device_id, int channel_id,
                               int *_func, bool *_connected) {
        *_func = func;
        *_connected = online;

        return new supla_channel_value();
      });

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(StrEq(expectedPayload)))
      .Times(1);

  supla_state_webhook_request2 *request = new supla_state_webhook_request2(
      supla_caller(ctDevice), 1, 2, 123, ET_CHANNEL_VALUE_CHANGED, 0, queue,
      pool, propertyGetter, &credentials);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 1000);
}

TEST_F(StateWebhookRequestTest, sendLightSwitchReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"LIGHTSWITCH\",\"timestamp\":"
      "1600097258,\"state\":{\"on\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, true,
           new supla_channel_onoff_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendLightSwitchReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"LIGHTSWITCH\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendPowerSwitchReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"POWERSWITCH\",\"timestamp\":"
      "1600097258,\"state\":{\"on\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_POWERSWITCH, true,
           new supla_channel_onoff_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendPowerSwitchReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"POWERSWITCH\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_POWERSWITCH, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendTemperatureReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"THERMOMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"temperature\":55.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_THERMOMETER, true,
           new supla_channel_temphum_value(false, 55.55, -1), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendTemperatureReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"THERMOMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_THERMOMETER, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendHumidityReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"HUMIDITY\",\"timestamp\":"
      "1600097258,\"state\":{\"humidity\":66.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_HUMIDITY, true,
           new supla_channel_temphum_value(true, 0, 66.55), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendHumidityReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"HUMIDITY\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_HUMIDITY, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendTemperatureAndHumidityReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"HUMIDITYANDTEMPERATURE\","
      "\"timestamp\":1600097258,\"state\":{\"temperature\":11.11,\"humidity\":"
      "22.22,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, true,
           new supla_channel_temphum_value(true, 11.11, 22.22),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendTemperatureAndHumidityReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"HUMIDITYANDTEMPERATURE\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendGatewayOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_GATEWAY\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendGatewayOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_GATEWAY\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendGatewOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_GATE\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_GATE, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendGateOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_GATE\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_GATE, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendGarageDoorOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_GARAGEDOOR\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendGarageDoorOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_GARAGEDOOR\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendNoLiquidSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"NOLIQUIDSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_NOLIQUIDSENSOR, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendNoLiquidSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"NOLIQUIDSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_NOLIQUIDSENSOR, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDoorOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_DOOR\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDoorOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_DOOR\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendRollerShutterOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_ROLLERSHUTTER\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendRollerShutterOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_ROLLERSHUTTER\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRoofWindowOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_ROOFWINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendRoofWindowOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_ROOFWINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWindowOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_WINDOW\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWindowOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_WINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendMailSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"MAILSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_MAILSENSOR, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendMailSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"MAILSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_MAILSENSOR, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRollerShutterReport_Calibration) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"CONTROLLINGTHEROLLERSHUTTER\","
      "\"timestamp\":1600097258,\"state\":{\"shut\":0,\"is_calibrating\":true,"
      "\"connected\":true}}";

  TDSC_RollerShutterValue rs_val = {};
  rs_val.position = -1;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
           new supla_channel_rs_value(&rs_val), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRollerShutterReport_PartlyClosed) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"CONTROLLINGTHEROLLERSHUTTER\","
      "\"timestamp\":1600097258,\"state\":{\"shut\":25,\"is_calibrating\":"
      "false,\"connected\":true}}";

  TDSC_RollerShutterValue rs_val = {};
  rs_val.position = 25;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
           new supla_channel_rs_value(&rs_val), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRollerShutterReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"CONTROLLINGTHEROLLERSHUTTER\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRoofWindowReport_Calibration) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"CONTROLLINGTHEROOFWINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"shut\":0,\"is_calibrating\":true,"
      "\"connected\":true}}";

  TDSC_RollerShutterValue rs_val = {};
  rs_val.position = -1;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW, true,
           new supla_channel_rs_value(&rs_val), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRoofWindowReport_PartlyClosed) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"CONTROLLINGTHEROOFWINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"shut\":25,\"is_calibrating\":"
      "false,\"connected\":true}}";

  TDSC_RollerShutterValue rs_val = {};
  rs_val.position = 25;

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW, true,
           new supla_channel_rs_value(&rs_val), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRoofWindowReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"CONTROLLINGTHEROOFWINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWindSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"WINDSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"value\":55.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_WINDSENSOR, true,
           new supla_channel_floating_point_sensor_value(55.55),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWindSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"WINDSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_WINDSENSOR, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendPressureSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"PRESSURESENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"value\":55.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_PRESSURESENSOR, true,
           new supla_channel_floating_point_sensor_value(55.55),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendPressureSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"PRESSURESENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_PRESSURESENSOR, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRainSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"RAINSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"value\":55.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_RAINSENSOR, true,
           new supla_channel_floating_point_sensor_value(55.55),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRainSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"RAINSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_RAINSENSOR, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWeightSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"WEIGHTSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"value\":55.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_WEIGHTSENSOR, true,
           new supla_channel_floating_point_sensor_value(55.55),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWeightSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"WEIGHTSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_WEIGHTSENSOR, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDistanceSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DISTANCESENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"distance\":55.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_DISTANCESENSOR, true,
           new supla_channel_floating_point_sensor_value(55.55),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDistanceSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DISTANCESENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_DISTANCESENSOR, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDepthSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DEPTHSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"depth\":55.55,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_DEPTHSENSOR, true,
           new supla_channel_floating_point_sensor_value(55.55),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDepthSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DEPTHSENSOR\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_DEPTHSENSOR, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDimmerReport_On) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DIMMER\",\"timestamp\":"
      "1600097258,\"state\":{\"brightness\":20,\"on\":true,\"connected\":true}"
      "}";

  TRGBW_Value rgbw = {};
  rgbw.brightness = 20;

  makeTest(SUPLA_CHANNELFNC_DIMMER, true, new supla_channel_rgbw_value(&rgbw),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDimmerReport_Off) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DIMMER\",\"timestamp\":"
      "1600097258,\"state\":{\"brightness\":0,\"on\":false,\"connected\":true}"
      "}";

  TRGBW_Value rgbw = {};

  makeTest(SUPLA_CHANNELFNC_DIMMER, true, new supla_channel_rgbw_value(&rgbw),
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDimmerReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DIMMER\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}"
      "}";

  makeTest(SUPLA_CHANNELFNC_DIMMER, true, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRgbReport_On) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"RGBLIGHTING\",\"timestamp\":"
      "1600097258,\"state\":{\"color\":\"0xAABBCC\",\"color_brightness\":20,"
      "\"on\":true,\"connected\":true}}";

  TRGBW_Value rgbw = {};
  rgbw.colorBrightness = 20;
  rgbw.R = 0xAA;
  rgbw.G = 0xBB;
  rgbw.B = 0xCC;

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRgbReport_Off) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"RGBLIGHTING\",\"timestamp\":"
      "1600097258,\"state\":{\"color\":\"0xDDEEFF\",\"color_brightness\":0,"
      "\"on\":false,\"connected\":true}}";

  TRGBW_Value rgbw = {};
  rgbw.colorBrightness = 0;
  rgbw.R = 0xDD;
  rgbw.G = 0xEE;
  rgbw.B = 0xFF;

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendRgbReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"RGBLIGHTING\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_RGBLIGHTING, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDimmerAndRgbReport_On) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DIMMERANDRGBLIGHTING\","
      "\"timestamp\":1600097258,\"state\":{\"color\":\"0xAABBCC\",\"color_"
      "brightness\":20,\"brightness\":40,\"on\":true,\"connected\":true}}";

  TRGBW_Value rgbw = {};
  rgbw.colorBrightness = 20;
  rgbw.brightness = 40;
  rgbw.R = 0xAA;
  rgbw.G = 0xBB;
  rgbw.B = 0xCC;

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDimmerAndRgbReport_Off) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DIMMERANDRGBLIGHTING\","
      "\"timestamp\":1600097258,\"state\":{\"color\":\"0x344556\",\"color_"
      "brightness\":0,\"brightness\":0,\"on\":false,\"connected\":true}}";

  TRGBW_Value rgbw = {};
  rgbw.R = 0x34;
  rgbw.G = 0x45;
  rgbw.B = 0x56;

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, true,
           new supla_channel_rgbw_value(&rgbw), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendDimmerAndRgbReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"DIMMERANDRGBLIGHTING\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterHeatMeasurementReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_HEATMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"totalCost\":555.5,\"pricePerUnit\":0.5555,"
      "\"impulsesPerUnit\":1000,\"counter\":1000000,\"calculatedValue\":1000,"
      "\"currency\":\"PLN\",\"unit\":\"m3\",\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  char currency[] = "PLN";
  char unit[] = "m3";

  supla_channel_ic_measurement *icm = new supla_channel_ic_measurement(
      123, SUPLA_CHANNELFNC_IC_HEAT_METER, &ic_val, currency, unit, 5555, 1000);

  makeTest(SUPLA_CHANNELFNC_IC_HEAT_METER, true, icm, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterHeatMeasurementReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_HEATMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_IC_HEAT_METER, false,
           (supla_channel_ic_measurement *)nullptr, expectedPayload);
}

}  // namespace testing
