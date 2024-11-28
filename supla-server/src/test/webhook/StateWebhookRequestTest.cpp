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

#include "StateWebhookRequestTest.h"

#include <memory>

#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_hvac_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "http/asynctask_http_thread_bucket.h"
#include "webhook/state_webhook_request.h"

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

  EXPECT_CALL(*curlAdapter,
              set_opt_url(Eq(0), StrEq("https://webhook.test.io/endpoint")))
      .Times(1);

  EXPECT_CALL(*curlAdapter,
              append_header(Eq(0), StrEq("Content-Type: application/json")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(
      *curlAdapter,
      append_header(Eq(0), StrEq("Authorization: Bearer MyAccessTokenXYZ")))
      .Times(1)
      .WillOnce(Return(true));

  EXPECT_CALL(*curlAdapter, perform).Times(1).WillOnce(Return(true));

  ON_CALL(*curlAdapter, get_response_code).WillByDefault(Return(200));
}

void StateWebhookRequestTest::makeTest(int func, bool online,
                                       supla_channel_value *value,
                                       const char *expectedPayload) {
  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(123), NotNull(), NotNull()))
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

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 123, 0, queue, pool, propertyGetter,
      &credentials);
  request->set_timestamp(1600097258);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

void StateWebhookRequestTest::makeTest(
    int func, bool online, supla_channel_extended_value *extended_value,
    const char *expectedPayload) {
  EXPECT_CALL(*propertyGetter, _get_extended_value(Eq(1), Eq(2), Eq(123)))
      .Times(1)
      .WillOnce(Return(extended_value));

  EXPECT_CALL(*propertyGetter,
              _get_value(Eq(1), Eq(2), Eq(123), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([func, online](int user_id, int device_id, int channel_id,
                               supla_channel_fragment *_fragment,
                               supla_channel_availability_status *_status) {
        *_fragment =
            supla_channel_fragment(device_id, channel_id, 0, 0, func, 0, false);
        _status->set_offline(!online);

        return new supla_channel_value();
      });

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 123, 0, queue, pool, propertyGetter,
      &credentials);
  request->set_timestamp(1600097258);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
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
      "1600097258,\"state\":{\"on\":false,\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_LIGHTSWITCH, false, (supla_channel_value *)nullptr,
           expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendLightSwitchReport_Disconnected_With202Code) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"LIGHTSWITCH\",\"timestamp\":"
      "1600097258,\"state\":{\"on\":false,\"connected\":false}}";

  EXPECT_CALL(*curlAdapter, get_response_code).WillRepeatedly(Return(202));

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
      "1600097258,\"state\":{\"on\":false,\"connected\":false}}";

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
      "1600097258,\"state\":{\"temperature\":-273,\"connected\":false}}";

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
      "1600097258,\"state\":{\"humidity\":-1,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"temperature\":-273,\"humidity\":-"
      "1,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

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
      "1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWindowOpeningSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_WINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendWindowOpeningSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"OPENINGSENSOR_WINDOW\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW, false,
           (supla_channel_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendHotelCardSensorReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"HOTELCARDSENSOR\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":true,\"connected\":true}}";

  makeTest(SUPLA_CHANNELFNC_HOTELCARDSENSOR, true,
           new supla_channel_binary_sensor_value(true), expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendHotelCardSensorReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"HOTELCARDSENSOR\","
      "\"timestamp\":1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_HOTELCARDSENSOR, false,
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
      "1600097258,\"state\":{\"hi\":false,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"shut\":0,\"is_calibrating\":false,"
      "\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"shut\":0,\"is_calibrating\":false,"
      "\"connected\":false}}";

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
      "1600097258,\"state\":{\"value\":0,\"connected\":false}}";

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
      "1600097258,\"state\":{\"value\":0,\"connected\":false}}";

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
      "1600097258,\"state\":{\"value\":0,\"connected\":false}}";

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
      "1600097258,\"state\":{\"value\":0,\"connected\":false}}";

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
      "1600097258,\"state\":{\"distance\":0,\"connected\":false}}";

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
      "1600097258,\"state\":{\"depth\":0,\"connected\":false}}";

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
      "1600097258,\"state\":{\"brightness\":0,\"on\":false,\"connected\":false}"
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
      "1600097258,\"state\":{\"color\":\"0x000000\",\"color_brightness\":0,"
      "\"on\":false,\"connected\":false}}";

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
      "\"timestamp\":1600097258,\"state\":{\"color\":\"0x000000\",\"color_"
      "brightness\":0,\"brightness\":0,\"on\":false,\"connected\":false}}";

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
      "\"currency\":\"PLN\",\"unit\":\"GJ\",\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  supla_json_config cfg;
  cfg.set_user_config(
      "{\"currency\":\"PLN\",\"unit\":\"GJ\",\"pricePerUnit\":5555,"
      "\"impulsesPerUnit\":1000}");

  supla_channel_ic_extended_value *icv = new supla_channel_ic_extended_value(
      SUPLA_CHANNELFNC_IC_HEAT_METER, &ic_val, &cfg);

  makeTest(SUPLA_CHANNELFNC_IC_HEAT_METER, true, icv, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterHeatMeasurementReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_HEATMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_IC_HEAT_METER, false,
           (supla_channel_ic_extended_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterGasMeasurementReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_GASMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"totalCost\":555.5,\"pricePerUnit\":0.5555,"
      "\"impulsesPerUnit\":1000,\"counter\":1000000,\"calculatedValue\":1000,"
      "\"currency\":\"PLN\",\"unit\":\"m3\",\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  supla_json_config cfg;
  cfg.set_user_config(
      "{\"currency\":\"PLN\",\"unit\":\"m3\",\"pricePerUnit\":5555,"
      "\"impulsesPerUnit\":1000}");

  supla_channel_ic_extended_value *icv = new supla_channel_ic_extended_value(
      SUPLA_CHANNELFNC_IC_GAS_METER, &ic_val, &cfg);

  makeTest(SUPLA_CHANNELFNC_IC_GAS_METER, true, icv, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterGasMeasurementReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_GASMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_IC_GAS_METER, false,
           (supla_channel_ic_extended_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterWaterMeasurementReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_WATERMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"totalCost\":555.5,\"pricePerUnit\":0.5555,"
      "\"impulsesPerUnit\":1000,\"counter\":1000000,\"calculatedValue\":1000,"
      "\"currency\":\"PLN\",\"unit\":\"m3\",\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  supla_json_config cfg;
  cfg.set_user_config(
      "{\"currency\":\"PLN\",\"unit\":\"m3\",\"pricePerUnit\":5555,"
      "\"impulsesPerUnit\":1000}");

  supla_channel_ic_extended_value *icv = new supla_channel_ic_extended_value(
      SUPLA_CHANNELFNC_IC_WATER_METER, &ic_val, &cfg);

  makeTest(SUPLA_CHANNELFNC_IC_WATER_METER, true, icv, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterWaterMeasurementReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_WATERMETER\",\"timestamp\":"
      "1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_IC_WATER_METER, false,
           (supla_channel_ic_extended_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterElectricityMeasurementReport_Connected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_ELECTRICITYMETER\","
      "\"timestamp\":1600097258,\"state\":{\"totalCost\":555.5,"
      "\"pricePerUnit\":0.5555,\"impulsesPerUnit\":1000,\"counter\":1000000,"
      "\"calculatedValue\":1000,\"currency\":\"PLN\",\"unit\":\"kWh\","
      "\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  supla_json_config cfg;
  cfg.set_user_config(
      "{\"currency\":\"PLN\",\"unit\":\"kWh\",\"pricePerUnit\":5555,"
      "\"impulsesPerUnit\":1000}");

  supla_channel_ic_extended_value *icv = new supla_channel_ic_extended_value(
      SUPLA_CHANNELFNC_IC_ELECTRICITY_METER, &ic_val, &cfg);

  makeTest(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER, true, icv, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendImpulseCounterElectricityMeasurementReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"IC_ELECTRICITYMETER\","
      "\"timestamp\":1600097258,\"state\":{\"connected\":false}}";

  makeTest(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER, false,
           (supla_channel_ic_extended_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendElectricityMeasurementReport_PowerActive_kW) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"ELECTRICITYMETER\","
      "\"timestamp\":1600097258,\"state\":{\"support\":7405567,\"currency\":"
      "\"PLN\",\"pricePerUnit\":1,\"totalCost\":3,\"phases\":[{\"number\":1,"
      "\"frequency\":60.01,\"voltage\":240,\"current\":50,\"powerActive\":1,"
      "\"powerReactive\":-1,\"powerApparent\":1,\"powerFactor\":1,"
      "\"phaseAngle\":-180,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1},{\"number\":2,\"frequency\":60.01,"
      "\"voltage\":240,\"current\":50,\"powerActive\":1,\"powerReactive\":-1,"
      "\"powerApparent\":1,\"powerFactor\":1,\"phaseAngle\":-180,"
      "\"totalForwardActiveEnergy\":1,\"totalReverseActiveEnergy\":1,"
      "\"totalForwardReactiveEnergy\":1,\"totalReverseReactiveEnergy\":1},{"
      "\"number\":3,\"frequency\":60.01,\"voltage\":240,\"current\":50,"
      "\"powerActive\":1,\"powerReactive\":-1,\"powerApparent\":1,"
      "\"powerFactor\":1,\"phaseAngle\":-180,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1}],\"totalForwardActiveEnergyBalanced\":"
      "1,\"totalReverseActiveEnergyBalanced\":1,\"connected\":true}}";

  TElectricityMeter_ExtendedValue_V3 em_ev = {};

  em_ev.m[0].freq = 6001;

  for (int p = 0; p < 3; p++) {
    em_ev.total_forward_active_energy[p] = 100000;
    em_ev.total_reverse_active_energy[p] = 100000;
    em_ev.total_forward_reactive_energy[p] = 100000;
    em_ev.total_reverse_reactive_energy[p] = 100000;

    em_ev.m[0].voltage[p] = 24000;
    em_ev.m[0].current[p] = 50000;

    em_ev.m[0].power_active[p] = 100;
    em_ev.m[0].power_reactive[p] = -100;
    em_ev.m[0].power_apparent[p] = 100;
    em_ev.m[0].power_factor[p] = 1000;
    em_ev.m[0].phase_angle[p] = -1800;
  }

  em_ev.total_forward_active_energy_balanced = 100000;
  em_ev.total_reverse_active_energy_balanced = 100000;

  em_ev.measured_values = 0xFFFF | EM_VAR_POWER_ACTIVE_KW |
                          EM_VAR_POWER_REACTIVE_KVAR |
                          EM_VAR_POWER_APPARENT_KVA;
  em_ev.period = 1;
  em_ev.m_count = 1;

  char currency[] = "PLN";

  supla_channel_em_extended_value *emv =
      new supla_channel_em_extended_value(&em_ev, currency, 10000);

  makeTest(SUPLA_CHANNELFNC_ELECTRICITY_METER, true, emv, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendElectricityMeasurementReport_AllVars) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{\"support\":"
      "65535,\"currency\":\"PLN\",\"pricePerUnit\":1,\"totalCost\":3,"
      "\"phases\":[{\"number\":1,\"frequency\":60.01,\"voltage\":240,"
      "\"current\":50,\"powerActive\":1,\"powerReactive\":-1,\"powerApparent\":"
      "1,\"powerFactor\":1,\"phaseAngle\":-180,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1},{\"number\":2,\"frequency\":60.01,"
      "\"voltage\":240,\"current\":50,\"powerActive\":1,\"powerReactive\":-1,"
      "\"powerApparent\":1,\"powerFactor\":1,\"phaseAngle\":-180,"
      "\"totalForwardActiveEnergy\":1,\"totalReverseActiveEnergy\":1,"
      "\"totalForwardReactiveEnergy\":1,\"totalReverseReactiveEnergy\":1},{"
      "\"number\":3,\"frequency\":60.01,\"voltage\":240,\"current\":50,"
      "\"powerActive\":1,\"powerReactive\":-1,\"powerApparent\":1,"
      "\"powerFactor\":1,\"phaseAngle\":-180,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1}],\"totalForwardActiveEnergyBalanced\":"
      "1,\"totalReverseActiveEnergyBalanced\":1,\"connected\":true}}";

  TElectricityMeter_ExtendedValue_V3 em_ev = {};

  em_ev.m[0].freq = 6001;

  for (int p = 0; p < 3; p++) {
    em_ev.total_forward_active_energy[p] = 100000;
    em_ev.total_reverse_active_energy[p] = 100000;
    em_ev.total_forward_reactive_energy[p] = 100000;
    em_ev.total_reverse_reactive_energy[p] = 100000;

    em_ev.m[0].voltage[p] = 24000;
    em_ev.m[0].current[p] = 50000;

    em_ev.m[0].power_active[p] = 100000;
    em_ev.m[0].power_reactive[p] = -100000;
    em_ev.m[0].power_apparent[p] = 100000;
    em_ev.m[0].power_factor[p] = 1000;
    em_ev.m[0].phase_angle[p] = -1800;
  }

  em_ev.total_forward_active_energy_balanced = 100000;
  em_ev.total_reverse_active_energy_balanced = 100000;

  em_ev.measured_values = 0xFFFF;
  em_ev.period = 1;
  em_ev.m_count = 1;

  char currency[] = "PLN";

  supla_channel_em_extended_value *em =
      new supla_channel_em_extended_value(&em_ev, currency, 10000);

  makeTest(SUPLA_CHANNELFNC_ELECTRICITY_METER, true, em, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendElectricityMeasurementReport_WithoutVoltageAndBalancedActiveEnergy) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{\"support\":"
      "57341,\"currency\":\"PLN\",\"pricePerUnit\":1,\"totalCost\":3,"
      "\"phases\":[{\"number\":1,\"frequency\":60.01,"
      "\"current\":50,\"powerActive\":1,\"powerReactive\":-1,\"powerApparent\":"
      "1,\"powerFactor\":1,\"phaseAngle\":-180,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1},{\"number\":2,\"frequency\":60.01,"
      "\"current\":50,\"powerActive\":1,\"powerReactive\":-1,"
      "\"powerApparent\":1,\"powerFactor\":1,\"phaseAngle\":-180,"
      "\"totalForwardActiveEnergy\":1,\"totalReverseActiveEnergy\":1,"
      "\"totalForwardReactiveEnergy\":1,\"totalReverseReactiveEnergy\":1},{"
      "\"number\":3,\"frequency\":60.01,\"current\":50,"
      "\"powerActive\":1,\"powerReactive\":-1,\"powerApparent\":1,"
      "\"powerFactor\":1,\"phaseAngle\":-180,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1}],\"totalReverseActiveEnergyBalanced\":"
      "1,\"connected\":true}}";

  TElectricityMeter_ExtendedValue_V3 em_ev = {};

  em_ev.m[0].freq = 6001;

  for (int p = 0; p < 3; p++) {
    em_ev.total_forward_active_energy[p] = 100000;
    em_ev.total_reverse_active_energy[p] = 100000;
    em_ev.total_forward_reactive_energy[p] = 100000;
    em_ev.total_reverse_reactive_energy[p] = 100000;

    em_ev.m[0].voltage[p] = 24000;
    em_ev.m[0].current[p] = 50000;

    em_ev.m[0].power_active[p] = 100000;
    em_ev.m[0].power_reactive[p] = -100000;
    em_ev.m[0].power_apparent[p] = 100000;
    em_ev.m[0].power_factor[p] = 1000;
    em_ev.m[0].phase_angle[p] = -1800;
  }

  em_ev.total_forward_active_energy_balanced = 100000;
  em_ev.total_reverse_active_energy_balanced = 100000;

  em_ev.measured_values = 0xFFFF;
  em_ev.measured_values ^= EM_VAR_VOLTAGE;
  em_ev.measured_values ^= EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;
  em_ev.period = 1;
  em_ev.m_count = 1;

  char currency[] = "PLN";

  supla_channel_em_extended_value *em =
      new supla_channel_em_extended_value(&em_ev, currency, 10000);

  makeTest(SUPLA_CHANNELFNC_ELECTRICITY_METER, true, em, expectedPayload);
}

TEST_F(StateWebhookRequestTest, sendElectricityMeasurementReport_Disconnected) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{\"connected\":"
      "false}}";

  makeTest(SUPLA_CHANNELFNC_ELECTRICITY_METER, false,
           (supla_channel_em_extended_value *)nullptr, expectedPayload);
}

TEST_F(StateWebhookRequestTest,
       sendElectricityMeasurementReport_LackOfMeasurements) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{\"support\":"
      "65535,\"currency\":\"PLN\",\"pricePerUnit\":1,\"totalCost\":3,"
      "\"phases\":[{\"number\":1,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1},{\"number\":2,"
      "\"totalForwardActiveEnergy\":1,\"totalReverseActiveEnergy\":1,"
      "\"totalForwardReactiveEnergy\":1,\"totalReverseReactiveEnergy\":1},{"
      "\"number\":3,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1}],\"totalForwardActiveEnergyBalanced\":"
      "1,\"totalReverseActiveEnergyBalanced\":1,\"connected\":true}}";

  TElectricityMeter_ExtendedValue_V3 em_ev = {};

  for (int p = 0; p < 3; p++) {
    em_ev.total_forward_active_energy[p] = 100000;
    em_ev.total_reverse_active_energy[p] = 100000;
    em_ev.total_forward_reactive_energy[p] = 100000;
    em_ev.total_reverse_reactive_energy[p] = 100000;
  }

  em_ev.total_forward_active_energy_balanced = 100000;
  em_ev.total_reverse_active_energy_balanced = 100000;

  em_ev.measured_values = 0xFFFF;
  em_ev.period = 1;

  char currency[] = "PLN";

  supla_channel_em_extended_value *em =
      new supla_channel_em_extended_value(&em_ev, currency, 10000);

  makeTest(SUPLA_CHANNELFNC_ELECTRICITY_METER, true, em, expectedPayload);
}

TEST_F(StateWebhookRequestTest, triggeredActionsReport_ToggleX1_PressX3) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":567,\"channelFunction\":\"ACTION_TRIGGER\",\"timestamp\":"
      "1600097258,\"triggered_actions\":[\"TOGGLE_X1\",\"PRESS_X3\"]}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 567,
      SUPLA_ACTION_CAP_TOGGLE_x1 | SUPLA_ACTION_CAP_SHORT_PRESS_x3, queue, pool,
      propertyGetter, &credentials);
  request->set_timestamp(1600097258);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(StateWebhookRequestTest, triggeredActionsReport_Hold) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":567,\"channelFunction\":\"ACTION_TRIGGER\",\"timestamp\":"
      "1600097258,\"triggered_actions\":[\"HOLD\"]}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 567, SUPLA_ACTION_CAP_HOLD, queue, pool,
      propertyGetter, &credentials);
  request->set_timestamp(1600097258);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(StateWebhookRequestTest, triggeredActionsReport_All) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":7777,\"channelFunction\":\"ACTION_TRIGGER\",\"timestamp\":"
      "1600097258,\"triggered_actions\":[\"TURN_ON\",\"TURN_OFF\",\"TOGGLE_"
      "X1\",\"TOGGLE_X2\",\"TOGGLE_X3\",\"TOGGLE_X4\",\"TOGGLE_X5\",\"HOLD\","
      "\"PRESS_X1\",\"PRESS_X2\",\"PRESS_X3\",\"PRESS_X4\",\"PRESS_X5\"]}";

  EXPECT_CALL(*curlAdapter, set_opt_post_fields(Eq(0), StrEq(expectedPayload)))
      .Times(1);

  supla_state_webhook_request *request = new supla_state_webhook_request(
      supla_caller(ctDevice), 1, 2, 7777, 0xFFFFFFFF, queue, pool,
      propertyGetter, &credentials);
  request->set_timestamp(1600097258);
  std::shared_ptr<supla_abstract_asynctask> task = request->start();
  WaitForState(task, supla_asynctask_state::SUCCESS, 10000);
}

TEST_F(StateWebhookRequestTest, hvacReport) {
  const char expectedPayload[] =
      "{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-e8be5c71ae5b\","
      "\"channelId\":123,\"channelFunction\":\"HVAC_THERMOSTAT\",\"timestamp\":"
      "1600097258,\"state\":{\"is_on\":true,\"mode\":\"HEAT_COOL\",\"setpoint_"
      "temperature_heat\":22.1,\"setpoint_temperature_cool\":19.55,\"flags\":["
      "\"HEATING\",\"FORCED_OFF_BY_SENSOR\"],\"connected\":true}}";

  supla_channel_hvac_value *hvac_val = new supla_channel_hvac_value();

  hvac_val->set_mode(SUPLA_HVAC_MODE_HEAT_COOL);
  hvac_val->set_setpoint_temperature_heat(2210);
  hvac_val->set_setpoint_temperature_cool(1955);
  hvac_val->set_flags(SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET |
                      SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET |
                      SUPLA_HVAC_VALUE_FLAG_HEATING |
                      SUPLA_HVAC_VALUE_FLAG_FORCED_OFF_BY_SENSOR);

  makeTest(SUPLA_CHANNELFNC_HVAC_THERMOSTAT, true, hvac_val, expectedPayload);
}

}  // namespace testing
