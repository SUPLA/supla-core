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

#include "AlexaClientTest.h"
#include "amazon/alexaclient.h"
#include "gtest/gtest.h"  // NOLINT

namespace {

class AlexaClientTest : public STTrivialHttp {
 protected:
  supla_alexa_client *client;

 public:
  void SetUp() override {
    getUser()->amazonAlexa()->set("ACCESS-TOKEN", "RERESH-TOKEN", 3600, "eu");
    client = new supla_alexa_client(getUser()->amazonAlexa());
  }

  void TearDown() override {
    delete client;
    client = NULL;
  }
};

TEST_F(AlexaClientTest, sendPowerChangeReport) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 909\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-1\"},\"payload\":{\"change\":{"
      "\"cause\":{\"type\":\"APP_INTERACTION\"},\"properties\":[{\"namespace\":"
      "\"Alexa.PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  ASSERT_TRUE(
      client->sendPowerChangeReport(CAUSE_APP_INTERACTION, 1, true, true));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 473\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{},\"event\":{\"header\":{\"messageId\":"
      "\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\","
      "\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-1\"},\"payload\":{\"change\":{\"cause\":{"
      "\"type\":\"APP_INTERACTION\"},\"properties\":[{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":"
      "\"UNREACHABLE\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  ASSERT_TRUE(
      client->sendPowerChangeReport(CAUSE_APP_INTERACTION, 1, true, false));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));
}

TEST_F(AlexaClientTest, sendContactChangeReport) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 478\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{},\"event\":{\"header\":{\"messageId\":"
      "\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\","
      "\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-2\"},\"payload\":{\"change\":{\"cause\":{"
      "\"type\":\"PHYSICAL_INTERACTION\"},\"properties\":[{\"namespace\":"
      "\"Alexa.EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":"
      "\"UNREACHABLE\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";

  ASSERT_TRUE(client->sendContactChangeReport(CAUSE_PHYSICAL_INTERACTION, 2,
                                              true, false));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 923\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ContactSensor\",\"name\":\"detectionState\",\"value\":\"DETECTED\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-2\"},\"payload\":{\"change\":{"
      "\"cause\":{\"type\":\"PERIODIC_POLL\"},\"properties\":[{\"namespace\":"
      "\"Alexa.ContactSensor\",\"name\":\"detectionState\",\"value\":"
      "\"DETECTED\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";

  ASSERT_TRUE(
      client->sendContactChangeReport(CAUSE_PERIODIC_POLL, 2, false, true));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));
}

TEST_F(AlexaClientTest, sendBrightnessChangeReport) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 470\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{},\"event\":{\"header\":{\"messageId\":"
      "\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\","
      "\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-3\"},\"payload\":{\"change\":{\"cause\":{"
      "\"type\":\"RULE_TRIGGER\"},\"properties\":[{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":"
      "\"UNREACHABLE\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";
  ASSERT_TRUE(
      client->sendBrightnessChangeReport(CAUSE_RULE_TRIGGER, 3, 10, false, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 1197\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
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
  ASSERT_TRUE(client->sendBrightnessChangeReport(CAUSE_VOICE_INTERACTION, 3, 10,
                                                 true, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 1201\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
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
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-4-"
      "1\"},\"payload\":{\"change\":{\"cause\":{\"type\":\"VOICE_INTERACTION\"}"
      ",\"properties\":[{\"namespace\":\"Alexa.BrightnessController\",\"name\":"
      "\"brightness\",\"value\":100,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";
  ASSERT_TRUE(client->sendBrightnessChangeReport(CAUSE_VOICE_INTERACTION, 4,
                                                 100, true, 1));
  ASSERT_TRUE(outputEqualTo(expectedRequest3));
}

TEST_F(AlexaClientTest, sendColorChangeReport) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 470\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{},\"event\":{\"header\":{\"messageId\":"
      "\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\","
      "\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-3\"},\"payload\":{\"change\":{\"cause\":{"
      "\"type\":\"RULE_TRIGGER\"},\"properties\":[{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":"
      "\"UNREACHABLE\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";
  ASSERT_TRUE(client->sendColorChangeReport(CAUSE_RULE_TRIGGER, 3, 0xFFAABB, 10,
                                            false, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 1581\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "ColorController\",\"name\":\"color\",\"value\":{\"hue\":348,"
      "\"saturation\":0.33333333333333331,\"brightness\":0.1},\"timeOfSample\":"
      "\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":50},{"
      "\"namespace\":\"Alexa.BrightnessController\",\"name\":\"brightness\","
      "\"value\":10,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-3\"},\"payload\":{\"change\":{"
      "\"cause\":{\"type\":\"VOICE_INTERACTION\"},\"properties\":[{"
      "\"namespace\":\"Alexa.ColorController\",\"name\":\"color\",\"value\":{"
      "\"hue\":348,\"saturation\":0.33333333333333331,\"brightness\":0.1},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.BrightnessController\",\"name\":"
      "\"brightness\",\"value\":10,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";
  ASSERT_TRUE(client->sendColorChangeReport(CAUSE_VOICE_INTERACTION, 3,
                                            0xFFAABB, 10, true, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 1545\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
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
      "\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":\"qwerty-4-"
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
  ASSERT_TRUE(client->sendColorChangeReport(CAUSE_VOICE_INTERACTION, 4,
                                            0x00FF00, 100, true, 1));
  ASSERT_TRUE(outputEqualTo(expectedRequest3));
}

TEST_F(AlexaClientTest, sendPercentageChangeReport) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 470\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{},\"event\":{\"header\":{\"messageId\":"
      "\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":\"Alexa\","
      "\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},\"endpoint\":{"
      "\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},"
      "\"endpointId\":\"qwerty-1\"},\"payload\":{\"change\":{\"cause\":{"
      "\"type\":\"RULE_TRIGGER\"},\"properties\":[{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":"
      "\"UNREACHABLE\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]}}}}";
  ASSERT_TRUE(
      client->sendPercentageChangeReport(CAUSE_RULE_TRIGGER, 1, 50, false));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 917\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "PercentageController\",\"name\":\"percentage\",\"value\":60,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.EndpointHealth\",\"name\":\"connectivity\","
      "\"value\":{\"value\":\"OK\"},\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"ChangeReport\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-1\"},\"payload\":{\"change\":{"
      "\"cause\":{\"type\":\"VOICE_INTERACTION\"},\"properties\":[{"
      "\"namespace\":\"Alexa.PercentageController\",\"name\":\"percentage\","
      "\"value\":60,\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50},{\"namespace\":\"Alexa."
      "EndpointHealth\",\"name\":\"connectivity\",\"value\":{\"value\":\"OK\"},"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]}}}}";
  ASSERT_TRUE(
      client->sendPercentageChangeReport(CAUSE_VOICE_INTERACTION, 1, 60, true));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));
}

TEST_F(AlexaClientTest, powerControllerSendResponse) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 384\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{\"message\":"
      "\"Unable to reach channel ID15 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";
  ASSERT_TRUE(
      client->powerControllerSendResponse("correlationToken", 15, true, false));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 442\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "PowerController\",\"name\":\"powerState\",\"value\":\"ON\","
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"Response\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-15\"},\"payload\":{}}}";
  ASSERT_TRUE(
      client->powerControllerSendResponse("correlationToken", 15, true, true));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));
}

TEST_F(AlexaClientTest, brightnessControllerSendResponse) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 382\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-3\"},\"payload\":{\"message\":\"Unable "
      "to reach channel ID3 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";
  ASSERT_TRUE(client->brightnessControllerSendResponse("correlationToken", 3,
                                                       10, false, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 584\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":10,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-3\"},\"payload\":{}}}";
  ASSERT_TRUE(client->brightnessControllerSendResponse("correlationToken", 3,
                                                       10, true, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 587\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "BrightnessController\",\"name\":\"brightness\",\"value\":100,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50},{\"namespace\":\"Alexa.PowerController\",\"name\":\"powerState\","
      "\"value\":\"ON\",\"timeOfSample\":\"2019-02-01T12:09:33Z\","
      "\"uncertaintyInMilliseconds\":50}]},\"event\":{\"header\":{"
      "\"messageId\":\"29012dd1-33c7-6519-6e18-c4ee71d00487\",\"namespace\":"
      "\"Alexa\",\"name\":\"Response\",\"correlationToken\":"
      "\"correlationToken\",\"payloadVersion\":\"3\"},\"endpoint\":{\"scope\":{"
      "\"type\":\"BearerToken\",\"token\":\"ACCESS-TOKEN\"},\"endpointId\":"
      "\"qwerty-4-1\"},\"payload\":{}}}";
  ASSERT_TRUE(client->brightnessControllerSendResponse("correlationToken", 4,
                                                       100, true, 1));
  ASSERT_TRUE(outputEqualTo(expectedRequest3));
}

TEST_F(AlexaClientTest, colorControllerSendResponse) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 382\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-3\"},\"payload\":{\"message\":\"Unable "
      "to reach channel ID3 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";
  ASSERT_TRUE(client->colorControllerSendResponse("correlationToken", 3,
                                                  0xFFAABB, 10, false, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 776\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
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
      "TOKEN\"},\"endpointId\":\"qwerty-3\"},\"payload\":{}}}";
  ASSERT_TRUE(client->colorControllerSendResponse("correlationToken", 3,
                                                  0xFFAABB, 10, true, 0));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));

  const char expectedRequest3[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 759\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
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
      "\"qwerty-4-1\"},\"payload\":{}}}";
  ASSERT_TRUE(client->colorControllerSendResponse("correlationToken", 4,
                                                  0x00FF00, 100, true, 1));
  ASSERT_TRUE(outputEqualTo(expectedRequest3));
}

TEST_F(AlexaClientTest, percentageControllerSendResponse) {
  const char expectedRequest1[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 382\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-"
      "6e18-c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"ErrorResponse\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-1\"},\"payload\":{\"message\":\"Unable "
      "to reach channel ID1 because it appears to be "
      "offline.\",\"type\":\"ENDPOINT_UNREACHABLE\"}}}";
  ASSERT_TRUE(client->percentageControllerSendResponse("correlationToken", 1,
                                                       50, false));
  ASSERT_TRUE(outputEqualTo(expectedRequest1));

  const char expectedRequest2[] =
      "POST /v3/events HTTP/1.1\r\nHost: api.eu.amazonalexa.com\r\nUser-Agent: "
      "supla-server\r\nContent-Length: 444\r\nAuthorization: Bearer "
      "ACCESS-TOKEN\r\nConnection: close\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"context\":{\"properties\":[{\"namespace\":\"Alexa."
      "PercentageController\",\"name\":\"percentage\",\"value\":60,"
      "\"timeOfSample\":\"2019-02-01T12:09:33Z\",\"uncertaintyInMilliseconds\":"
      "50}]},\"event\":{\"header\":{\"messageId\":\"29012dd1-33c7-6519-6e18-"
      "c4ee71d00487\",\"namespace\":\"Alexa\",\"name\":\"Response\","
      "\"correlationToken\":\"correlationToken\",\"payloadVersion\":\"3\"},"
      "\"endpoint\":{\"scope\":{\"type\":\"BearerToken\",\"token\":\"ACCESS-"
      "TOKEN\"},\"endpointId\":\"qwerty-1\"},\"payload\":{}}}";
  ASSERT_TRUE(client->percentageControllerSendResponse("correlationToken", 1,
                                                       60, true));
  ASSERT_TRUE(outputEqualTo(expectedRequest2));
}

}  // namespace
