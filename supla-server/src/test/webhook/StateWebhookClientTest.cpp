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

#include "StateWebhookClientTest.h"
#include "TrivialHttpFactoryMock.h"
#include "TrivialHttpMock.h"
#include "webhook/statewebhookcredentials.h"

namespace testing {

StateWebhookClientTest::StateWebhookClientTest() {}

void StateWebhookClientTest::SetUp() {
  supla_user::init();
  user = new supla_user(1001);
  user->setUniqueId("dc85740d-cb27-405b-9da3-e8be5c71ae5b", "");

  user->stateWebhookCredentials()->set("ACCESS-TOKEN", "RERESH-TOKEN", 3600,
                                       "https://localhost", "130,140");
  client = new supla_state_webhook_client(user->stateWebhookCredentials());

  client->setHttpConnectionFactory(new TrivialHttpFactoryMock());
}
void StateWebhookClientTest::TearDown() {
  delete client->getHttpConnectionFactory();
  delete client;
  delete user;

  supla_user::user_free();
}

TEST_F(StateWebhookClientTest, sendLightSwitchReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 168\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"LIGHTSWITCH\",\"timestamp\":1600097258,\"state\":{\"on\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendLightSwitchReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 170\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"LIGHTSWITCH\",\"timestamp\":1600097258,\"state\":{\"on\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendLightSwitchReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendPowerSwitchReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 168\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"POWERSWITCH\",\"timestamp\":1600097258,\"state\":{\"on\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendPowerSwitchReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 170\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"POWERSWITCH\",\"timestamp\":1600097258,\"state\":{\"on\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendPowerSwitchReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendTemperatureReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 178\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"THERMOMETER\",\"timestamp\":1600097258,\"state\":{\"temperature\":"
      "55.55,\"connected\":true}}";

  ASSERT_TRUE(client->sendTemperatureReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 179\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"THERMOMETER\",\"timestamp\":1600097258,\"state\":{\"temperature\":"
      "44.44,\"connected\":false}}";

  ASSERT_TRUE(client->sendTemperatureReport(123, 44.44, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendHumidityReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"HUMIDITY\",\"timestamp\":1600097258,\"state\":{\"humidity\":66.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendHumidityReport(123, 66.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 173\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"HUMIDITY\",\"timestamp\":1600097258,\"state\":{\"humidity\":50.44,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendHumidityReport(123, 50.44, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendTemperatureAndHumidityReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 206\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"HUMIDITYANDTEMPERATURE\",\"timestamp\":1600097258,\"state\":{"
      "\"temperature\":11.11,\"humidity\":22.22,\"connected\":true}}";

  ASSERT_TRUE(
      client->sendTemperatureAndHumidityReport(123, 11.11, 22.22, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 207\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"HUMIDITYANDTEMPERATURE\",\"timestamp\":1600097258,\"state\":{"
      "\"temperature\":33.33,\"humidity\":44.44,\"connected\":false}}";

  ASSERT_TRUE(
      client->sendTemperatureAndHumidityReport(123, 33.33, 44.44, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendGatewayOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 178\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_GATEWAY\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendGatewayOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 180\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_GATEWAY\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendGatewayOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendGateOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 175\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_GATE\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendGateOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 177\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_GATE\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendGateOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendGarageDoorOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 181\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_GARAGEDOOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendGarageDoorOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 183\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_GARAGEDOOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendGarageDoorOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendNoLiquidSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 171\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"NOLIQUIDSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendNoLiquidSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 173\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"NOLIQUIDSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendNoLiquidSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendDoorOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 175\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_DOOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendDoorOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 177\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_DOOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendDoorOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendRollerShutterOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 184\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_ROLLERSHUTTER\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendRollerShutterOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 186\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_ROLLERSHUTTER\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendRollerShutterOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendWindowOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 177\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_WINDOW\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendWindowOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 179\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"OPENINGSENSOR_WINDOW\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendWindowOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendMailSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 167\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"MAILSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendMailSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 169\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"MAILSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendMailSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendRollerShutterReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 205\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"CONTROLLINGTHEROLLERSHUTTER\",\"timestamp\":1600097258,\"state\":{"
      "\"shut\":0,\"connected\":true,\"is_calibrating\":true}}";

  ASSERT_TRUE(client->sendRollerShutterReport(123, -1, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 207\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"CONTROLLINGTHEROLLERSHUTTER\",\"timestamp\":1600097258,\"state\":{"
      "\"shut\":25,\"connected\":true,\"is_calibrating\":false}}";

  ASSERT_TRUE(client->sendRollerShutterReport(123, 25, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 207\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"CONTROLLINGTHEROLLERSHUTTER\",\"timestamp\":1600097258,\"state\":{"
      "\"shut\":5,\"connected\":false,\"is_calibrating\":false}}";

  ASSERT_TRUE(client->sendRollerShutterReport(123, 5, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
}

TEST_F(StateWebhookClientTest, sendWindSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 171\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"WINDSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendWindSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"WINDSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendWindSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendPressureSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 175\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"PRESSURESENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendPressureSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 176\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"PRESSURESENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendPressureSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendRainSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 171\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"RAINSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendRainSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"RAINSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendRainSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendWeightSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 173\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"WEIGHTSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendWeightSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 174\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"WEIGHTSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"value\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendWeightSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendDistanceSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 178\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DISTANCESENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"distance\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendDistanceSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 179\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DISTANCESENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"distance\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendDistanceSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendDepthSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DEPTHSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"depth\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendDepthSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 173\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DEPTHSENSOR\",\"timestamp\":1600097258,\"state\":{"
      "\"depth\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendDepthSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendDimmerReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 179\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DIMMER\",\"timestamp\":1600097258,\"state\":{"
      "\"brightness\":20,\"on\":true,\"connected\":true}}";

  ASSERT_TRUE(client->sendDimmerReport(123, 20, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 180\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DIMMER\",\"timestamp\":1600097258,\"state\":{"
      "\"brightness\":30,\"on\":false,\"connected\":true}}";

  ASSERT_TRUE(client->sendDimmerReport(123, 30, false, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 181\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DIMMER\",\"timestamp\":1600097258,\"state\":{"
      "\"brightness\":10,\"on\":false,\"connected\":false}}";

  ASSERT_TRUE(client->sendDimmerReport(123, 10, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
}

TEST_F(StateWebhookClientTest, sendRgbReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 209\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"RGBLIGHTING\",\"timestamp\":1600097258,\"state\":{"
      "\"color\":\"0xAABBCC\",\"color_brightness\":20,\"on\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendRgbReport(123, 0xAABBCC, 20, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 210\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"RGBLIGHTING\",\"timestamp\":1600097258,\"state\":{"
      "\"color\":\"0xDDEEFF\",\"color_brightness\":10,\"on\":false,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendRgbReport(123, 0xDDEEFF, 10, false, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 210\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"RGBLIGHTING\",\"timestamp\":1600097258,\"state\":{"
      "\"color\":\"0x001122\",\"color_brightness\":5,\"on\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendRgbReport(123, 0x001122, 5, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
}

TEST_F(StateWebhookClientTest, sendDimmerAndRgbReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 234\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DIMMERANDRGBLIGHTING\",\"timestamp\":1600097258,\"state\":{"
      "\"color\":\"0xAABBCC\",\"color_brightness\":20,\"brightness\":40,\"on\":"
      "true,\"connected\":true}}";

  ASSERT_TRUE(
      client->sendDimmerAndRgbReport(123, 0xAABBCC, 20, 40, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 235\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DIMMERANDRGBLIGHTING\",\"timestamp\":1600097258,\"state\":{"
      "\"color\":\"0x334455\",\"color_brightness\":25,\"brightness\":45,\"on\":"
      "false,\"connected\":true}}";

  ASSERT_TRUE(
      client->sendDimmerAndRgbReport(123, 0x334455, 25, 45, false, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 236\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"DIMMERANDRGBLIGHTING\",\"timestamp\":1600097258,\"state\":{"
      "\"color\":\"0x344556\",\"color_brightness\":26,\"brightness\":46,\"on\":"
      "false,\"connected\":false}}";

  ASSERT_TRUE(
      client->sendDimmerAndRgbReport(123, 0x344556, 26, 46, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
}

TEST_F(StateWebhookClientTest, sendImpulseCounterHeatMeasurementReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 292\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_HEATMETER\",\"timestamp\":1600097258,\"state\":{\"totalCost\":555."
      "5,\"pricePerUnit\":0.5555,\"impulsesPerUnit\":1000,\"counter\":1000000,"
      "\"calculatedValue\":1000,\"currency\":\"PLN\",\"unit\":\"m3\","
      "\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  char currency[] = "PLN";
  char unit[] = "m3";

  supla_channel_ic_measurement icm(123, SUPLA_CHANNELFNC_IC_HEAT_METER, &ic_val,
                                   currency, unit, 5555, 1000);

  ASSERT_TRUE(client->sendImpulseCounterHeatMeasurementReport(123, &icm, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 160\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_HEATMETER\",\"timestamp\":1600097258,\"state\":{\"connected\":"
      "false}}";

  ASSERT_TRUE(
      client->sendImpulseCounterHeatMeasurementReport(123, &icm, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendImpulseCounterWaterMeasurementReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 293\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_WATERMETER\",\"timestamp\":1600097258,\"state\":{\"totalCost\":555."
      "5,\"pricePerUnit\":0.5555,\"impulsesPerUnit\":1000,\"counter\":1000000,"
      "\"calculatedValue\":1000,\"currency\":\"PLN\",\"unit\":\"m3\","
      "\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  char currency[] = "PLN";
  char unit[] = "m3";

  supla_channel_ic_measurement icm(123, SUPLA_CHANNELFNC_IC_WATER_METER,
                                   &ic_val, currency, unit, 5555, 1000);

  ASSERT_TRUE(
      client->sendImpulseCounterWaterMeasurementReport(123, &icm, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 161\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_WATERMETER\",\"timestamp\":1600097258,\"state\":{\"connected\":"
      "false}}";

  ASSERT_TRUE(
      client->sendImpulseCounterWaterMeasurementReport(123, &icm, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendImpulseCounterGasMeasurementReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 291\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_GASMETER\",\"timestamp\":1600097258,\"state\":{\"totalCost\":555."
      "5,\"pricePerUnit\":0.5555,\"impulsesPerUnit\":1000,\"counter\":1000000,"
      "\"calculatedValue\":1000,\"currency\":\"PLN\",\"unit\":\"m3\","
      "\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  char currency[] = "PLN";
  char unit[] = "m3";

  supla_channel_ic_measurement icm(123, SUPLA_CHANNELFNC_IC_GAS_METER, &ic_val,
                                   currency, unit, 5555, 1000);

  ASSERT_TRUE(client->sendImpulseCounterGasMeasurementReport(123, &icm, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 159\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_GASMETER\",\"timestamp\":1600097258,\"state\":{\"connected\":"
      "false}}";

  ASSERT_TRUE(client->sendImpulseCounterGasMeasurementReport(123, &icm, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendImpulseCounterElectricityMeasurementReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 300\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{"
      "\"totalCost\":555."
      "5,\"pricePerUnit\":0.5555,\"impulsesPerUnit\":1000,\"counter\":1000000,"
      "\"calculatedValue\":1000,\"currency\":\"PLN\",\"unit\":\"kWh\","
      "\"connected\":true}}";

  TDS_ImpulseCounter_Value ic_val;
  ic_val.counter = 1000000;

  char currency[] = "PLN";
  char unit[] = "kWh";

  supla_channel_ic_measurement icm(123, SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                   &ic_val, currency, unit, 5555, 1000);

  ASSERT_TRUE(
      client->sendImpulseCounterElectricityMeasurementReport(123, &icm, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 167\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"IC_ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{"
      "\"connected\":"
      "false}}";

  ASSERT_TRUE(
      client->sendImpulseCounterElectricityMeasurementReport(123, &icm, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendElectricityMeasurementReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 1107\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
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

  TElectricityMeter_ExtendedValue_V2 em_ev;
  memset(&em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V2));

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

  em_ev.measured_values = EM_VAR_ALL;
  em_ev.period = 1;
  em_ev.m_count = 1;

  char currency[] = "PLN";
  {
    supla_channel_electricity_measurement em(123, &em_ev, 10000, currency);

    ASSERT_TRUE(client->sendElectricityMeasurementReport(123, &em, true));
    ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));
  }

  em_ev.measured_values ^= EM_VAR_VOLTAGE;
  em_ev.measured_values ^= EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 1028\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
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

  {
    supla_channel_electricity_measurement em(123, &em_ev, 10000, currency);

    ASSERT_TRUE(client->sendElectricityMeasurementReport(123, &em, true));
    ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
  }

  const char expectedRequest3[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 164\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{\"connected\":"
      "false}}";

  {
    supla_channel_electricity_measurement em(123, &em_ev, 10000, currency);

    ASSERT_TRUE(client->sendElectricityMeasurementReport(123, &em, false));
    ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
  }

  em_ev.m_count = 0;

  const char expectedRequest4[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 674\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":123,\"channelFunction\":"
      "\"ELECTRICITYMETER\",\"timestamp\":1600097258,\"state\":{\"support\":"
      "57341,\"currency\":\"PLN\",\"pricePerUnit\":1,\"totalCost\":3,"
      "\"phases\":[{\"number\":1,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1},{\"number\":2,"
      "\"totalForwardActiveEnergy\":1,\"totalReverseActiveEnergy\":1,"
      "\"totalForwardReactiveEnergy\":1,\"totalReverseReactiveEnergy\":1},{"
      "\"number\":3,\"totalForwardActiveEnergy\":1,"
      "\"totalReverseActiveEnergy\":1,\"totalForwardReactiveEnergy\":1,"
      "\"totalReverseReactiveEnergy\":1}],\"totalReverseActiveEnergyBalanced\":"
      "1,\"connected\":true}}";

  {
    supla_channel_electricity_measurement em(123, &em_ev, 10000, currency);

    ASSERT_TRUE(client->sendElectricityMeasurementReport(123, &em, true));
    ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest4));
  }
}

} /* namespace testing */
