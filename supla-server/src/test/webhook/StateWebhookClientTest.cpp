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
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"LIGHTSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendLightSwitchReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 174\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"LIGHTSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendLightSwitchReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendPowerSwitchReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 172\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"POWERSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendPowerSwitchReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 174\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"POWERSWITCH\",\"timestamp\":\"1600097258\",\"state\":{\"on\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendPowerSwitchReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendTemperatureReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 182\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"THERMOMETER\",\"timestamp\":\"1600097258\",\"state\":{\"temperature\":"
      "55.55,\"connected\":true}}";

  ASSERT_TRUE(client->sendTemperatureReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 183\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"THERMOMETER\",\"timestamp\":\"1600097258\",\"state\":{\"temperature\":"
      "44.44,\"connected\":false}}";

  ASSERT_TRUE(client->sendTemperatureReport(123, 44.44, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendHumidityReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 176\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"HUMIDITY\",\"timestamp\":\"1600097258\",\"state\":{\"humidity\":66.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendHumidityReport(123, 66.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 177\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"HUMIDITY\",\"timestamp\":\"1600097258\",\"state\":{\"humidity\":50.44,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendHumidityReport(123, 50.44, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendTemperatureAndHumidityReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 210\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"HUMIDITYANDTEMPERATURE\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"temperature\":11.11,\"humidity\":22.22,\"connected\":true}}";

  ASSERT_TRUE(
      client->sendTemperatureAndHumidityReport(123, 11.11, 22.22, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 211\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"HUMIDITYANDTEMPERATURE\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"temperature\":33.33,\"humidity\":44.44,\"connected\":false}}";

  ASSERT_TRUE(
      client->sendTemperatureAndHumidityReport(123, 33.33, 44.44, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendGatewayOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 182\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_GATEWAY\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendGatewayOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 184\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_GATEWAY\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendGatewayOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendGateOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 179\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_GATE\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendGateOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 181\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_GATE\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendGateOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendGarageDoorOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 185\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_GARAGEDOOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendGarageDoorOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 187\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_GARAGEDOOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendGarageDoorOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendNoLiquidSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 175\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"NOLIQUIDSENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendNoLiquidSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 177\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"NOLIQUIDSENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendNoLiquidSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendDoorOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 179\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_DOOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendDoorOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 181\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_DOOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendDoorOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendRollerShutterOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 188\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_ROLLERSHUTTER\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendRollerShutterOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 190\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_ROLLERSHUTTER\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendRollerShutterOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendWindowOpeningSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 181\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_WINDOW\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendWindowOpeningSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 183\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"OPENINGSENSOR_WINDOW\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendWindowOpeningSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendMailSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 171\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"MAILSENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":true,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendMailSensorReport(123, true, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 173\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"MAILSENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"hi\":false,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendMailSensorReport(123, false, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendRollerShutterReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 209\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"CONTROLLINGTHEROLLERSHUTTER\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"shut\":0,\"connected\":true,\"is_calibrating\":true}}";

  ASSERT_TRUE(client->sendRollerShutterReport(123, -1, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 211\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"CONTROLLINGTHEROLLERSHUTTER\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"shut\":25,\"connected\":true,\"is_calibrating\":false}}";

  ASSERT_TRUE(client->sendRollerShutterReport(123, 25, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 211\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"CONTROLLINGTHEROLLERSHUTTER\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"shut\":5,\"connected\":false,\"is_calibrating\":false}}";

  ASSERT_TRUE(client->sendRollerShutterReport(123, 5, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
}

TEST_F(StateWebhookClientTest, sendWindSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 175\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"WINDSENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"value\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendWindSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 176\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"WINDSENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"value\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendWindSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(StateWebhookClientTest, sendPressureSensorReport) {
  const char expectedRequest1[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 179\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"PRESSURESENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"value\":55.55,"
      "\"connected\":true}}";

  ASSERT_TRUE(client->sendPressureSensorReport(123, 55.55, true));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 180\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"userShortUniqueId\":\"dc85740d-cb27-405b-9da3-"
      "e8be5c71ae5b\",\"channelId\":\"123\",\"channelFunction\":"
      "\"PRESSURESENSOR\",\"timestamp\":\"1600097258\",\"state\":{"
      "\"value\":66.66,"
      "\"connected\":false}}";

  ASSERT_TRUE(client->sendPressureSensorReport(123, 66.66, false));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

} /* namespace testing */
