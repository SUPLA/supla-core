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

#include "MqttSubscriberIntegrationTest.h"
#include "log.h"
#include "mqtt_subscriber.h"
#include "mqtt_subscriber_datasource.h"

namespace testing {

MqttSubscriberIntegrationTest::MqttSubscriberIntegrationTest()
    : MqttClientIntegrationTest() {
  value_setter = NULL;
}

MqttSubscriberIntegrationTest::~MqttSubscriberIntegrationTest() {}

void MqttSubscriberIntegrationTest::waitForData(int expectedTopicCount) {
  for (int a = 0; a < 5000; a++) {
    if (getLibAdapter()->subscribed_count() == expectedTopicCount &&
        !getDS()->is_context_open()) {
      return;
    }
    usleep(1000);
  }

  EXPECT_EQ(getLibAdapter()->subscribed_count(), expectedTopicCount);
  EXPECT_TRUE(getDS()->is_context_open());
  ASSERT_TRUE(false);
}

supla_mqtt_client *MqttSubscriberIntegrationTest::clientInit(
    supla_mqtt_client_library_adapter *library_adapter,
    supla_mqtt_client_settings *settings,
    supla_mqtt_client_datasource *datasource) {
  value_setter = new MqttChannelValueSetterMock(settings);
  return new supla_mqtt_subscriber(library_adapter, settings, datasource,
                                   value_setter);
}

void MqttSubscriberIntegrationTest::TearDown() {
  if (value_setter != NULL) {
    delete value_setter;
    value_setter = NULL;
  }

  MqttClientIntegrationTest::TearDown();
}

MqttChannelValueSetterMock *MqttSubscriberIntegrationTest::getValueSetter(
    void) {
  return value_setter;
}

supla_mqtt_client_datasource *MqttSubscriberIntegrationTest::dsInit(
    supla_mqtt_client_settings *settings) {
  return new supla_mqtt_subscriber_datasource(settings);
}

TEST_F(MqttSubscriberIntegrationTest, fullScope) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare(
                "supla/+/devices/+/channels/+/set/+"),
            0);
  ASSERT_EQ(getLibAdapter()->subscribed_pop().compare(
                "supla/+/devices/+/channels/+/execute_action"),
            0);
}

TEST_F(MqttSubscriberIntegrationTest, deviceAndChannelId) {
  waitForConnection();
  waitForData(2);

  ASSERT_TRUE(getValueSetter()->channelEqualTo(0));

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/5/channels/1/set/on", "1");

  ASSERT_TRUE(getValueSetter()->deviceEqualTo(5));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1));

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/56/channels/12/set/on", "12");

  ASSERT_TRUE(getValueSetter()->deviceEqualTo(56));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(12));

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/143/channels/123/set/on", "123");

  ASSERT_TRUE(getValueSetter()->deviceEqualTo(143));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(123));

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/560/channels/-1/set/on", "1");

  ASSERT_TRUE(getValueSetter()->deviceEqualTo(560));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(-1));
}

TEST_F(MqttSubscriberIntegrationTest, email) {
  waitForConnection();
  waitForData(2);

  ASSERT_TRUE(getValueSetter()->emailEqualTo(NULL));

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1/set/on", "1");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));

  getLibAdapter()->on_message_received("supla/a@b/devices/10/channels/1/set/on",
                                       "1");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("a@b"));

  getLibAdapter()->on_message_received(
      "supla/abcd/devices/10/channels/1/set/on", "1");

  ASSERT_TRUE(getValueSetter()->emailEqualTo(NULL));

  getLibAdapter()->on_message_received("supla/a@b/devices/0/channels/1/set/on",
                                       "1");

  ASSERT_TRUE(getValueSetter()->emailEqualTo(NULL));

  getLibAdapter()->on_message_received("supla/a@b/devices/10/channels/0/set/on",
                                       "1");

  ASSERT_TRUE(getValueSetter()->emailEqualTo(NULL));
}

TEST_F(MqttSubscriberIntegrationTest, setOnWithoutPrefix) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/on", "1");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOnCounter(), 1);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/9/channels/1234/set/on", "YeS");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOnCounter(), 2);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/on", "tRuE");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOnCounter(), 3);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/100/channels/1234/set/on", "tRuE!");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOnCounter(), 3);
}

TEST_F(MqttSubscriberIntegrationTest, setOnWithPrefix) {
  waitForConnection();
  waitForData(2);

  char prefix[] = "prefix/123";
  getSettings()->setPrefixPtr(prefix);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "prefix/123/supla/user@supla.org/devices/10/channels/1234/set/on", "1");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOnCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, setOff) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/500/channels/1234/set/on", "0");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOffCounter(), 1);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/100000/channels/1234/set/on", "nO");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOffCounter(), 2);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/on", "FaLsE");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOffCounter(), 3);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/on", "F!LsE");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOffCounter(), 3);
}

TEST_F(MqttSubscriberIntegrationTest, turnOn) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action",
      "TuRn_On");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOnCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, turnOff) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action",
      "TuRn_OfF");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOffCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, toggle) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action", "tOgGle");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getToggleCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, shut) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action", "sHuT");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getShutCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, reveal) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action", "rEvEal");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getRevealCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, stop) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action", "StOp");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getStopCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, open) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action", "oPeN");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOpenCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, openClose) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/execute_action",
      "oPeN_cLoSe");

  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getOpenCloseCounter(), 1);
}

TEST_F(MqttSubscriberIntegrationTest, setClosingPercentage) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), -1);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/closing_percentage",
      "34.56");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->getShutCounter(), 1);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 34);

  getValueSetter()->clear();

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/closing_percentage",
      "-10");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/closing_percentage",
      "0");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 1);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/closing_percentage",
      "33");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 2);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 33);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/closing_percentage",
      "100");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 3);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 100);

  getValueSetter()->clear();

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/closing_percentage",
      "110");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 0);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), -1);
}

TEST_F(MqttSubscriberIntegrationTest, setOpeningPercentage) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), -1);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/opening_percentage",
      "45.6789");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->getShutCounter(), 1);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 55);

  getValueSetter()->clear();

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/opening_percentage",
      "-10");

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/opening_percentage",
      "0");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 1);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 100);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/opening_percentage",
      "33");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 2);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 67);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/opening_percentage",
      "100");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 3);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), 0);

  getValueSetter()->clear();

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/opening_percentage",
      "110");

  ASSERT_EQ(getValueSetter()->getShutCounter(), 0);
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);
  ASSERT_EQ(getValueSetter()->getClosingPercentage(), -1);
}

TEST_F(MqttSubscriberIntegrationTest, setBrightness) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);
  ASSERT_EQ(getValueSetter()->getBrightness(), -1);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/brightness",
      "33.33abcd");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/brightness", "-1");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/brightness", "110");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/brightness", "12.34");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getBrightnessCounter(), 1);
  ASSERT_EQ(getValueSetter()->getBrightness(), 12);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/brightness", "0");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getBrightnessCounter(), 2);
  ASSERT_EQ(getValueSetter()->getBrightness(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/brightness", "100");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getBrightnessCounter(), 3);
  ASSERT_EQ(getValueSetter()->getBrightness(), 100);
}

TEST_F(MqttSubscriberIntegrationTest, setColorBrightness) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);
  ASSERT_EQ(getValueSetter()->getColorBrightness(), -1);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color_brightness",
      "33,33");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color_brightness",
      "-1");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color_brightness",
      "101");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color_brightness",
      "12.34");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorBrightnessCounter(), 1);
  ASSERT_EQ(getValueSetter()->getColorBrightness(), 12);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color_brightness",
      "0");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorBrightnessCounter(), 2);
  ASSERT_EQ(getValueSetter()->getColorBrightness(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color_brightness",
      "100");

  ASSERT_TRUE(getValueSetter()->channelEqualTo(1234));
  ASSERT_TRUE(getValueSetter()->emailEqualTo("user@supla.org"));
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorBrightnessCounter(), 3);
  ASSERT_EQ(getValueSetter()->getColorBrightness(), 100);
}

TEST_F(MqttSubscriberIntegrationTest, setColor) {
  waitForConnection();
  waitForData(2);

  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0x01FFFFFF);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "0xFF00FFa");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "#FF00FFa");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "0xFF00FG");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "#FF00FG");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "#FFFFFF");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 1);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xFFFFFF);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "#000000");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 2);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "#AABBCC");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 3);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xAABBCC);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "0xFFFFFF");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 4);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xFFFFFF);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "0x000000");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 5);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "0xAABBCC");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 6);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xAABBCC);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "0xAA05CC");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 7);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xAA05CC);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "255,255,255");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 8);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xFFFFFF);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "0,0,0");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 9);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0x000000);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "170,187,204");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 10);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xAABBCC);

  getLibAdapter()->on_message_received(
      "supla/user@supla.org/devices/10/channels/1234/set/color", "170,5,204");
  ASSERT_EQ(getValueSetter()->counterSetCount(), 1);
  ASSERT_EQ(getValueSetter()->getColorCounter(), 11);
  ASSERT_EQ(getValueSetter()->getColor(), (unsigned int)0xAA05CC);
}

} /* namespace testing */
