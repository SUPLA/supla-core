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

#include "MqttStateMessageProviderTest.h"

namespace testing {

MqttStateMessageProviderTest::MqttStateMessageProviderTest(void)
    : MqttMessageProviderTest() {}
MqttStateMessageProviderTest::~MqttStateMessageProviderTest(void) {}

void MqttStateMessageProviderTest::SetUp() {
  provider = new MqttStateMessageProviderMock();
}

void MqttStateMessageProviderTest::TearDown() { delete provider; }

TEST_F(MqttStateMessageProviderTest, dataExistsMethodTest) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
  cvalue.hi = false;
  cvalue.shut = -1;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  for (int a = 0; a < 4; a++) {
    ASSERT_TRUE(dataExists(provider));
  }

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, rollerShutterConnected) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
  cvalue.hi = false;
  cvalue.shut = -1;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "false", false,
                              "/user@supla.org/channels/%i/state/hi", 789));

  ASSERT_TRUE(
      fetchAndCompare(provider, "/%email%", "true", false,
                      "/user@supla.org/channels/%i/state/is_calibrating", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "0", false,
                              "/user@supla.org/channels/%i/state/shut", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, rollerShutterDisconnected) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = false;
  cvalue.function = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
  cvalue.hi = false;
  cvalue.shut = -1;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "false", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", NULL, false,
                              "/user@supla.org/channels/%i/state/hi", 789));

  ASSERT_TRUE(
      fetchAndCompare(provider, "/%email%", NULL, false,
                      "/user@supla.org/channels/%i/state/is_calibrating", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", NULL, false,
                              "/user@supla.org/channels/%i/state/shut", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, gate) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_CONTROLLINGTHEGATE;
  cvalue.hi = false;
  cvalue.partially_closed = true;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "false", false,
                              "/user@supla.org/channels/%i/state/hi", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/partial_hi",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, lock) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK;
  cvalue.hi = true;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/hi", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, onOff) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_POWERSWITCH;
  cvalue.hi = true;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/on", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, depth) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DEPTHSENSOR;
  cvalue.depth = 345;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "345.000000", false,
                              "/user@supla.org/channels/%i/state/depth", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, distance) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DISTANCESENSOR;
  cvalue.distance = 1345;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "1345.000000", false,
                              "/user@supla.org/channels/%i/state/distance",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, wightSensor) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_WEIGHTSENSOR;
  cvalue.weight = 85;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "85.000000", false,
                              "/user@supla.org/channels/%i/state/value", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, garageDoorSensor) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR;
  cvalue.hi = true;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/hi", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, thermometer) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_THERMOMETER;
  cvalue.temperature = 33;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "33.000000", false,
                              "/user@supla.org/channels/%i/state/temperature",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, humidity) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_HUMIDITY;
  cvalue.humidity = 85;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "85.000000", false,
                              "/user@supla.org/channels/%i/state/humidity",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, temperatureAndHumidity) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;
  cvalue.temperature = 21;
  cvalue.humidity = 83;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "83.000000", false,
                              "/user@supla.org/channels/%i/state/humidity",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "21.000000", false,
                              "/user@supla.org/channels/%i/state/temperature",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerOn) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DIMMER;
  cvalue.brightness = 99;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "99", false,
                              "/user@supla.org/channels/%i/state/brightness",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerOff) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DIMMER;
  cvalue.brightness = 0;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "false", false,
                              "/user@supla.org/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "0", false,
                              "/user@supla.org/channels/%i/state/brightness",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, rgb) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_RGBLIGHTING;
  cvalue.color = 0xAABBCC;
  cvalue.color_brightness = 40;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "0xAABBCC", false,
                              "/user@supla.org/channels/%i/state/color", 789));
  ASSERT_TRUE(fetchAndCompare(
      provider, "/%email%", "40", false,
      "/user@supla.org/channels/%i/state/color_brightness", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerAndRgb) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING;
  cvalue.color = 0xBBCCDD;
  cvalue.color_brightness = 0;
  cvalue.brightness = 80;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "0xBBCCDD", false,
                              "/user@supla.org/channels/%i/state/color", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, "/%email%", "0", false,
      "/user@supla.org/channels/%i/state/color_brightness", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "80", false,
                              "/user@supla.org/channels/%i/state/brightness",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, valveOpenClose) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_VALVE_OPENCLOSE;
  cvalue.valve_value.closed = true;
  cvalue.valve_value.flags =
      SUPLA_VALVE_FLAG_MANUALLY_CLOSED | SUPLA_VALVE_FLAG_FLOODING;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/closed", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, "/%email%", "true", false,
      "/user@supla.org/channels/%i/state/manually_closed", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/flooding",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, valvePercentage) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_VALVE_PERCENTAGE;
  cvalue.valve_value.closed = 81;

  provider->setComplexValue(&cvalue);
  provider->set_data("user@supla.org", 123, 456, 789);

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "true", false,
                              "/user@supla.org/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "81", false,
                              "/user@supla.org/channels/%i/state/closed", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, "/%email%", "false", false,
      "/user@supla.org/channels/%i/state/manually_closed", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "/%email%", "false", false,
                              "/user@supla.org/channels/%i/state/flooding",
                              789));

  ASSERT_FALSE(dataExists(provider));
}

} /* namespace testing */
