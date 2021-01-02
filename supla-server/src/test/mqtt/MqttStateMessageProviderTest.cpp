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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/user@supla.org/devices/456/channels/%i/state/hi", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/is_calibrating",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "0", false,
      "supla/user@supla.org/devices/456/channels/%i/state/shut", 789));

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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, NULL, false,
      "supla/user@supla.org/devices/456/channels/%i/state/hi", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, NULL, false,
      "supla/user@supla.org/devices/456/channels/%i/state/is_calibrating",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, NULL, false,
      "supla/user@supla.org/devices/456/channels/%i/state/shut", 789));

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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/user@supla.org/devices/456/channels/%i/state/hi", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/partial_hi", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, lock) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK;
  cvalue.hi = true;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/hi", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, onOff) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_POWERSWITCH;
  cvalue.hi = true;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/on", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, depth) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DEPTHSENSOR;
  cvalue.depth = 345;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "345.000000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/depth", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, distance) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DISTANCESENSOR;
  cvalue.distance = 1345;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1345.000000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/distance", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, wightSensor) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_WEIGHTSENSOR;
  cvalue.weight = 85;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "85.000000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/value", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, garageDoorSensor) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR;
  cvalue.hi = true;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/hi", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, thermometer) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_THERMOMETER;
  cvalue.temperature = 33;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "33.000000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/temperature", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, humidity) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_HUMIDITY;
  cvalue.humidity = 85;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "85.000000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/humidity", 789));

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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "83.000000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/humidity", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "21.000000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/temperature", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerOn) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DIMMER;
  cvalue.brightness = 99;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "99", false,
      "supla/user@supla.org/devices/456/channels/%i/state/brightness", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerOff) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_DIMMER;
  cvalue.brightness = 0;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/user@supla.org/devices/456/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "0", false,
      "supla/user@supla.org/devices/456/channels/%i/state/brightness", 789));

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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "0xAABBCC", false,
      "supla/user@supla.org/devices/456/channels/%i/state/color", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "170,187,204", false,
      "supla/user@supla.org/devices/456/channels/%i/state/normalized_rgb",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "40", false,
      "supla/user@supla.org/devices/456/channels/%i/state/color_brightness",
      789));

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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, "suffix", "true", false,
      "suffix/supla/user@supla.org/devices/456/channels/%i/state/connected",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, "suffix", "true", false,
      "suffix/supla/user@supla.org/devices/456/channels/%i/state/on", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, "suffix", "0xBBCCDD", false,
      "suffix/supla/user@supla.org/devices/456/channels/%i/state/color", 789));

  ASSERT_TRUE(fetchAndCompare(provider, "suffix", "0", false,
                              "suffix/supla/user@supla.org/devices/456/"
                              "channels/%i/state/color_brightness",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      provider, "suffix", "80", false,
      "suffix/supla/user@supla.org/devices/456/channels/%i/state/brightness",
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
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/closed", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/manually_closed",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/flooding", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, valvePercentage) {
  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));

  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_VALVE_PERCENTAGE;
  cvalue.valve_value.closed = 81;

  provider->setComplexValue(&cvalue);
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "81", false,
      "supla/user@supla.org/devices/456/channels/%i/state/closed", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/user@supla.org/devices/456/channels/%i/state/manually_closed",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "false", false,
      "supla/user@supla.org/devices/456/channels/%i/state/flooding", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, impulseCounter) {
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));
  cvalue.online = true;
  cvalue.function = SUPLA_CHANNELFNC_IC_GAS_METER;
  provider->setComplexValue(&cvalue);

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.23", false,
      "supla/user@supla.org/devices/456/channels/%i/state/total_cost", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.0000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/price_per_unit",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/impulses_per_unit",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1230", false,
      "supla/user@supla.org/devices/456/channels/%i/state/counter", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.230", false,
      "supla/user@supla.org/devices/456/channels/%i/state/calculated_value",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "EUR", false,
      "supla/user@supla.org/devices/456/channels/%i/state/currency", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "m3", false,
      "supla/user@supla.org/devices/456/channels/%i/state/unit", 789));

  ASSERT_FALSE(dataExists(provider));
}

TEST_F(MqttStateMessageProviderTest, electricityMeter) {
  provider->set_ids(123, 456, 789);
  provider->set_user_email();

  channel_complex_value cvalue;
  memset(&cvalue, 0, sizeof(channel_complex_value));
  cvalue.online = true;
  cvalue.channel_type = SUPLA_CHANNELTYPE_ELECTRICITY_METER;
  cvalue.function = SUPLA_CHANNELFNC_ELECTRICITY_METER;
  provider->setComplexValue(&cvalue);

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "true", false,
      "supla/user@supla.org/devices/456/channels/%i/state/connected", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "66.90", false,
      "supla/user@supla.org/devices/456/channels/%i/state/total_cost", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "69.90", false,
      "supla/user@supla.org/devices/456/channels/%i/state/total_cost_balanced",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.0000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/price_per_unit",
      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "EUR", false,
      "supla/user@supla.org/devices/456/channels/%i/state/currency", 789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "65535", false,
      "supla/user@supla.org/devices/456/channels/%i/state/support", 789));

  ASSERT_TRUE(
      fetchAndCompare(provider, NULL, "66.90000", false,
                      "supla/user@supla.org/devices/456/channels/%i/state/"
                      "total_forward_active_energy",
                      789));

  ASSERT_TRUE(
      fetchAndCompare(provider, NULL, "96.90000", false,
                      "supla/user@supla.org/devices/456/channels/%i/state/"
                      "total_reverse_active_energy",
                      789));

  ASSERT_TRUE(
      fetchAndCompare(provider, NULL, "69.90000", false,
                      "supla/user@supla.org/devices/456/channels/%i/state/"
                      "total_forward_active_energy_balanced",
                      789));

  ASSERT_TRUE(
      fetchAndCompare(provider, NULL, "99.90000", false,
                      "supla/user@supla.org/devices/456/channels/%i/state/"
                      "total_reverse_active_energy_balanced",
                      789));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "12.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/"
      "%i/total_forward_active_energy",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "22.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/"
      "%i/total_reverse_active_energy",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "32.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "total_forward_reactive_energy",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "42.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "total_reverse_reactive_energy",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "49.97", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "frequency",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "237.06", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "voltage",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.123", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "current",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_active",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "2.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_reactive",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "3.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_apparent",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.331", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_factor",
      789, 1));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "1.1", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "phase_angle",
      789, 1));

  //------------

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "22.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/"
      "%i/total_forward_active_energy",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "32.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/"
      "%i/total_reverse_active_energy",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "42.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "total_forward_reactive_energy",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "52.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "total_reverse_reactive_energy",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "49.97", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "frequency",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "337.06", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "voltage",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "2.123", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "current",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "2.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_active",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "3.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_reactive",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "4.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_apparent",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "2.331", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_factor",
      789, 2));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "2.1", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "phase_angle",
      789, 2));

  //------------

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "32.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/"
      "%i/total_forward_active_energy",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "42.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/"
      "%i/total_reverse_active_energy",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "52.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "total_forward_reactive_energy",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "62.30000", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "total_reverse_reactive_energy",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "49.97", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "frequency",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "437.06", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "voltage",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "3.123", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "current",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "3.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_active",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "4.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_reactive",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "5.11223", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_apparent",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "3.331", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "power_factor",
      789, 3));

  ASSERT_TRUE(fetchAndCompare(
      provider, NULL, "3.1", false,
      "supla/user@supla.org/devices/456/channels/%i/state/phases/%i/"
      "phase_angle",
      789, 3));

  ASSERT_FALSE(dataExists(provider));
}

} /* namespace testing */
