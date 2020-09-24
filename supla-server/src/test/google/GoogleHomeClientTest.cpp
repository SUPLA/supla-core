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

#include "GoogleHomeClientTest.h"
#include "TrivialHttpFactoryMock.h"
#include "TrivialHttpMock.h"
#include "gtest/gtest.h"  // NOLINT

namespace testing {

GoogleHomeClientTest::GoogleHomeClientTest() {}

void GoogleHomeClientTest::SetUp() {
  supla_user::init();
  user = new supla_user(1001);
  user->setUniqueId("qwerty", "zxcvbnm");

  user->googleHomeCredentials()->set("ACCESS-TOKEN");
  client = new supla_google_home_client(user->googleHomeCredentials());

  client->setHttpConnectionFactory(new TrivialHttpFactoryMock());
}
void GoogleHomeClientTest::TearDown() {
  delete client->getHttpConnectionFactory();
  delete client;
  delete user;

  supla_user::user_free();
}
TEST_F(GoogleHomeClientTest, requestSync) {
  const char expectedRequest[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 107\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\","
      "\"agentUserId\":\"zxcvbnm\",\"intent\":\"action.devices.SYNC\"}";

  ASSERT_TRUE(client->requestSync());
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest));
}

TEST_F(GoogleHomeClientTest, onOffRepoerState) {
  const char expectedRequest1[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 151\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\","
      "\"agentUserId\":\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{"
      "\"qwerty-10\":{\"online\":false,\"on\":false}}}}}";

  ASSERT_TRUE(client->addOnOffState(10, true, false));
  ASSERT_TRUE(client->sendReportState(NULL));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 118\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"on\":true}}}}}";

  ASSERT_TRUE(client->addOnOffState(10, true, true));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));
}

TEST_F(GoogleHomeClientTest, brightnessState) {
  const char expectedRequest1[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 166\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\","
      "\"agentUserId\":\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{"
      "\"qwerty-10\":{\"online\":false,\"on\":false,\"brightness\":0}}}}}";

  ASSERT_TRUE(client->addBrightnessState(10, 55, false, 0));
  ASSERT_TRUE(client->sendReportState(NULL));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 134\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"on\":true,\"brightness\":55}}}}}";

  ASSERT_TRUE(client->addBrightnessState(10, 55, true, 0));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 136\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-10-1\":{\"online\":true,"
      "\"on\":true,\"brightness\":80}}}}}";

  ASSERT_TRUE(client->addBrightnessState(10, 80, true, 1));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
}

TEST_F(GoogleHomeClientTest, colorState) {
  const char expectedRequest1[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 199\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\","
      "\"agentUserId\":\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{"
      "\"qwerty-10\":{\"online\":false,\"color\":{\"spectrumRGB\":16711867},"
      "\"on\":false,\"brightness\":0}}}}}";

  ASSERT_TRUE(client->addColorState(10, 0xFF00BB, 55, false, 0));
  ASSERT_TRUE(client->sendReportState(NULL));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 166\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"color\":{\"spectrumRGB\":4486946},\"on\":true,\"brightness\":55}}}}}";

  ASSERT_TRUE(client->addColorState(10, 0x447722, 55, true, 0));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 169\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-10-1\":{\"online\":true,"
      "\"color\":{\"spectrumRGB\":16776960},\"on\":true,\"brightness\":80}}}}}";

  ASSERT_TRUE(client->addColorState(10, 0xFFFF00, 80, true, 1));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));
}

TEST_F(GoogleHomeClientTest, rollerShutterState) {
  const char expectedRequest1[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 169\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\","
      "\"agentUserId\":\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{"
      "\"qwerty-10\":{\"online\":false,\"on\":false,\"openPercent\":100}}}}}";

  ASSERT_TRUE(client->addRollerShutterState(10, 55, false));
  ASSERT_TRUE(client->sendReportState(NULL));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 135\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"on\":true,\"openPercent\":45}}}}}";

  ASSERT_TRUE(client->addRollerShutterState(10, 55, true));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 135\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-10\":{\"online\":true,"
      "\"on\":true,\"openPercent\":20}}}}}";

  ASSERT_TRUE(client->addRollerShutterState(10, 80, true));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest3));

  const char expectedRequest4[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 167\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"e2de5bc6-65a8-48e5-b919-8a48e86ad64a\","
      "\"agentUserId\":\"zxcvbnm\",\"payload\":{\"devices\":{\"states\":{"
      "\"qwerty-10\":{\"online\":true,\"on\":true,\"openPercent\":100}}}}}";

  ASSERT_TRUE(client->addRollerShutterState(10, 0, true));
  ASSERT_TRUE(client->sendReportState(NULL));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest4));
}

TEST_F(GoogleHomeClientTest, sendFullReportState) {
  const char expectedRequest[] =
      "POST /default/googleHomeGraphBridge HTTP/1.1\r\nHost: "
      "2rxqysinpg.execute-api.eu-west-1.amazonaws.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 453\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: "
      "close\r\n\r\n{\"requestId\":\"REQID\",\"agentUserId\":\"zxcvbnm\","
      "\"payload\":{\"devices\":{\"states\":{\"qwerty-1\":{\"online\":true,"
      "\"on\":true},\"qwerty-2\":{\"online\":true,\"color\":{\"spectrumRGB\":"
      "16776960},\"on\":true,\"brightness\":80},\"qwerty-3-1\":{\"online\":"
      "true,\"color\":{\"spectrumRGB\":12320512},\"on\":true,\"brightness\":80}"
      ",\"qwerty-4\":{\"online\":true,\"on\":true,\"brightness\":55},\"qwerty-"
      "5-2\":{\"online\":true,\"on\":true,\"brightness\":30},\"qwerty-6\":{"
      "\"online\":true,\"on\":true,\"openPercent\":70}}}}}";

  ASSERT_TRUE(client->addOnOffState(1, true, true));
  ASSERT_FALSE(client->addOnOffState(1, true, true));
  ASSERT_TRUE(client->addColorState(2, 0xFFFF00, 80, true, 0));
  ASSERT_FALSE(client->addColorState(2, 0xFFFF00, 80, true, 0));
  ASSERT_TRUE(client->addColorState(3, 0xBBFF00, 80, true, 1));
  ASSERT_FALSE(client->addColorState(3, 0xBBFF00, 80, true, 1));
  ASSERT_TRUE(client->addBrightnessState(4, 55, true, 0));
  ASSERT_FALSE(client->addBrightnessState(4, 55, true, 0));
  ASSERT_TRUE(client->addBrightnessState(5, 30, true, 2));
  ASSERT_FALSE(client->addBrightnessState(5, 30, true, 2));
  ASSERT_TRUE(client->addRollerShutterState(6, 30, true));
  ASSERT_FALSE(client->addRollerShutterState(6, 45, true));
  ASSERT_TRUE(client->sendReportState("REQID"));
  ASSERT_TRUE(TrivialHttpMock::outputEqualTo(expectedRequest));
}

} /* namespace testing */
