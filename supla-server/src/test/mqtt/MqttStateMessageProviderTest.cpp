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

#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_gate_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_openclosed_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_valve_value.h"

namespace testing {

void MqttStateMessageProviderTest::SetUp() {
  Test::SetUp();
  short_unique_id = "9920767494dd87196e1896c7cbab707c";
  ON_CALL(provider, _get_user_suid)
      .WillByDefault(Return(short_unique_id.c_str()));
  ON_CALL(provider, _get_channel_property_getter)
      .WillByDefault(Return(&propertyGetter));

  provider.set_ids(123, 456, 789);
  provider.set_user_suid();
}

void MqttStateMessageProviderTest::SetResultValue(int func, bool connected,
                                                  supla_channel_value *value) {
  EXPECT_CALL(propertyGetter,
              _get_value(Eq(123), Eq(456), Eq(789), NotNull(), NotNull()))
      .Times(1)
      .WillOnce([func, connected, value](
                    int user_id, int device_id, int channel_id,
                    supla_channel_fragment *_fragment, bool *_connected) {
        *_fragment =
            supla_channel_fragment(device_id, channel_id, 0, func, 0, false);
        *_connected = connected;
        return value;
      });
}

void MqttStateMessageProviderTest::SetIcMeasurementResult(void) {
  EXPECT_CALL(propertyGetter, _get_extended_value(Eq(123), Eq(456), Eq(789)))
      .Times(1)
      .WillOnce([](int user_id, int device_id, int channel_id) {
        TDS_ImpulseCounter_Value ic_val;
        ic_val.counter = 1230;

        char customUnit[] = "m3";
        char currency[] = "EUR";

        return new supla_channel_ic_extended_value(
            SUPLA_CHANNELFNC_IC_GAS_METER, &ic_val, currency, customUnit, 10000,
            1000);
      });
}

void MqttStateMessageProviderTest::SetElectricityMeasurementResult(
    int measured_values, int m_count) {
  EXPECT_CALL(propertyGetter, _get_extended_value(Eq(123), Eq(456), Eq(789)))
      .Times(1)
      .WillOnce([measured_values, m_count](int user_id, int device_id,
                                           int channel_id) {
        TElectricityMeter_ExtendedValue_V2 em_ev;
        memset(&em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V2));

        em_ev.m[0].freq = 4997;

        for (short a = 0; a < 3; a++) {
          em_ev.m[0].voltage[a] = 23706 + a * 10000;
          em_ev.m[0].current[a] = 1123 + a * 1000;
          em_ev.m[0].power_active[a] = 111223 + a * 100000;
          em_ev.m[0].power_reactive[a] = 211223 + a * 100000;
          em_ev.m[0].power_apparent[a] = 311223 + a * 100000;
          em_ev.m[0].power_factor[a] = 1331 + a * 1000;
          em_ev.m[0].phase_angle[a] = 11 + a * 10;

          em_ev.total_forward_active_energy[a] = 1230000 + a * 1000000;
          em_ev.total_reverse_active_energy[a] = 2230000 + a * 1000000;
          em_ev.total_forward_reactive_energy[a] = 3230000 + a * 1000000;
          em_ev.total_reverse_reactive_energy[a] = 4230000 + a * 1000000;

          em_ev.total_forward_active_energy_balanced +=
              em_ev.total_forward_active_energy[a] + 100000;
          em_ev.total_reverse_active_energy_balanced +=
              em_ev.total_reverse_active_energy[a] + 100000;
        }

        em_ev.measured_values = measured_values;
        em_ev.period = 1;
        em_ev.m_count = m_count;

        char currency[] = "EUR";
        return new supla_channel_em_extended_value(&em_ev, currency, 10000);
      });
}

TEST_F(MqttStateMessageProviderTest, dataExistsMethodTest) {
  TDSC_RollerShutterValue rs = {};
  rs.position = -1;
  SetResultValue(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
                 new supla_channel_rs_value(&rs));

  for (int a = 0; a < 4; a++) {
    ASSERT_TRUE(dataExists(&provider));
  }

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, rollerShutterConnected) {
  TDSC_RollerShutterValue rs = {};
  rs.position = -1;
  SetResultValue(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, true,
                 new supla_channel_rs_value(&rs));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "false", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/hi",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/is_calibrating",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/shut",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, rollerShutterDisconnected) {
  TDSC_RollerShutterValue rs = {};
  rs.position = -1;
  SetResultValue(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER, false,
                 new supla_channel_rs_value(&rs));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "false", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, NULL, false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/hi",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/is_calibrating",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/shut",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, gate) {
  SetResultValue(SUPLA_CHANNELFNC_CONTROLLINGTHEGATE, true,
                 new supla_channel_gate_value(gsl_open, gsl_closed));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "false", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/hi",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/partial_hi",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, lock) {
  SetResultValue(SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK, true,
                 new supla_channel_openclosed_value(true));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "true", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/hi",
      789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, onOff) {
  SetResultValue(SUPLA_CHANNELFNC_POWERSWITCH, true,
                 new supla_channel_onoff_value(true));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "true", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/on",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "false", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/overcurrent_relay_off",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, overcurrentRelayOff) {
  supla_channel_onoff_value *onoff_val = new supla_channel_onoff_value();
  onoff_val->set_overcurrent_relay_off(true);
  SetResultValue(SUPLA_CHANNELFNC_POWERSWITCH, true, onoff_val);

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "false", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/on",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/overcurrent_relay_off",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, depth) {
  SetResultValue(SUPLA_CHANNELFNC_DEPTHSENSOR, true,
                 new supla_channel_floating_point_sensor_value(345));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "345.000000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/depth",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, distance) {
  SetResultValue(SUPLA_CHANNELFNC_DISTANCESENSOR, true,
                 new supla_channel_floating_point_sensor_value(1345));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1345.000000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/distance",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, wightSensor) {
  SetResultValue(SUPLA_CHANNELFNC_WEIGHTSENSOR, true,
                 new supla_channel_floating_point_sensor_value(85));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "85.000000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/value",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, garageDoorSensor) {
  SetResultValue(SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR, true,
                 new supla_channel_binary_sensor_value(true));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "true", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/hi",
      789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, thermometer) {
  SetResultValue(SUPLA_CHANNELFNC_THERMOMETER, true,
                 new supla_channel_temphum_value(false, 33.0, 0.0));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "33.000000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/temperature",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, humidity) {
  SetResultValue(SUPLA_CHANNELFNC_HUMIDITY, true,
                 new supla_channel_temphum_value(true, 0.0, 85));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "85.000000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/humidity",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, temperatureAndHumidity) {
  SetResultValue(SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE, true,
                 new supla_channel_temphum_value(true, 21.0, 83.0));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "83.000000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/humidity",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "21.000000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/temperature",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerOn) {
  TRGBW_Value rgbw = {};
  rgbw.brightness = 99;

  SetResultValue(SUPLA_CHANNELFNC_DIMMER, true,
                 new supla_channel_rgbw_value(&rgbw));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "true", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/on",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "99", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/brightness",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerOff) {
  TRGBW_Value rgbw = {};

  SetResultValue(SUPLA_CHANNELFNC_DIMMER, true,
                 new supla_channel_rgbw_value(&rgbw));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "false", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/on",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/brightness",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, rgb) {
  TRGBW_Value rgbw = {};
  rgbw.colorBrightness = 40;
  rgbw.R = 0xAA;
  rgbw.G = 0xBB;
  rgbw.B = 0xCC;

  SetResultValue(SUPLA_CHANNELFNC_RGBLIGHTING, true,
                 new supla_channel_rgbw_value(&rgbw));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "true", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/on",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0xAABBCC", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/color",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "170,187,204", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/normalized_rgb",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "40", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/color_brightness",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, dimmerAndRgb) {
  TRGBW_Value rgbw = {};
  rgbw.brightness = 80;
  rgbw.colorBrightness = 0;
  rgbw.R = 0xBB;
  rgbw.G = 0xCC;
  rgbw.B = 0xDD;

  SetResultValue(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING, true,
                 new supla_channel_rgbw_value(&rgbw));

  ASSERT_TRUE(fetchAndCompare(&provider, "suffix", "true", false,
                              "suffix/supla/9920767494dd87196e1896c7cbab707c/"
                              "devices/456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, "suffix", "true", false,
                              "suffix/supla/9920767494dd87196e1896c7cbab707c/"
                              "devices/456/channels/%i/state/on",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, "suffix", "0xBBCCDD", false,
                              "suffix/supla/9920767494dd87196e1896c7cbab707c/"
                              "devices/456/channels/%i/state/color",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, "suffix", "0", false,
      "suffix/supla/9920767494dd87196e1896c7cbab707c/devices/456/"
      "channels/%i/state/color_brightness",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, "suffix", "80", false,
                              "suffix/supla/9920767494dd87196e1896c7cbab707c/"
                              "devices/456/channels/%i/state/brightness",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, valveOpenClose) {
  TValve_Value vv = {};
  vv.closed = true;
  vv.flags = SUPLA_VALVE_FLAG_MANUALLY_CLOSED | SUPLA_VALVE_FLAG_FLOODING;

  SetResultValue(SUPLA_CHANNELFNC_VALVE_OPENCLOSE, true,
                 new supla_channel_valve_value(&vv));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/closed",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/manually_closed",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/flooding",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, valvePercentage) {
  TValve_Value vv = {};
  vv.closed = 81;

  SetResultValue(SUPLA_CHANNELFNC_VALVE_PERCENTAGE, true,
                 new supla_channel_valve_value(&vv));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "81", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/closed",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "false", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/manually_closed",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "false", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/flooding",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, impulseCounter) {
  SetResultValue(SUPLA_CHANNELFNC_IC_GAS_METER, true,
                 new supla_channel_value());

  SetIcMeasurementResult();

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.23", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/total_cost",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.0000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/price_per_unit",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/impulses_per_unit",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1230", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/counter",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.230", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/calculated_value",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "EUR", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/currency",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "m3", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/unit",
                              789));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, electricityMeter) {
  SetResultValue(SUPLA_CHANNELFNC_ELECTRICITY_METER, true,
                 new supla_channel_value());

  SetElectricityMeasurementResult(EM_VAR_ALL, 1);

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "66.90", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/total_cost",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "69.90", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/total_cost_balanced",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.0000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/price_per_unit",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "EUR", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/currency",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "65535", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/support",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "66.90000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_forward_active_energy",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "96.90000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_reverse_active_energy",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "69.90000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_forward_active_energy_balanced",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "99.90000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_reverse_active_energy_balanced",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "12.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "22.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "32.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "42.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "49.97", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "237.06", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.123", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "2.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.331", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.1", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 1));

  //------------

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "22.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "32.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "42.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "52.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "49.97", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "337.06", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "2.123", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "2.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "4.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "2.331", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "2.1", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 2));

  //------------

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "32.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "42.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "52.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "62.30000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "49.97", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "437.06", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3.123", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "4.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "5.11223", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3.331", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3.1", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 3));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, measurementUnitChangeTest) {
  short a = 0;

  SetResultValue(SUPLA_CHANNELFNC_ELECTRICITY_METER, true,
                 new supla_channel_value());

  SetElectricityMeasurementResult(EM_VAR_ALL | EM_VAR_POWER_ACTIVE_KW |
                                      EM_VAR_POWER_REACTIVE_KVAR |
                                      EM_VAR_POWER_APPARENT_KVA,
                                  1);

  for (a = 0; a < 17; a++) {
    ASSERT_TRUE(dataExists(&provider));
  }

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "2112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 1));
  for (a = 0; a < 9; a++) {
    ASSERT_TRUE(dataExists(&provider));
  }

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "2112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "4112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 2));

  for (a = 0; a < 9; a++) {
    ASSERT_TRUE(dataExists(&provider));
  }

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "3112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "4112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "5112.23000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 3));

  ASSERT_TRUE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, electricityMeterWithoutPhaseValues) {
  SetResultValue(SUPLA_CHANNELFNC_ELECTRICITY_METER, true,
                 new supla_channel_value());

  SetElectricityMeasurementResult(0, 1);

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "66.90", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/total_cost",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/total_cost_balanced",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "1.0000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/price_per_unit",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "EUR", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/currency",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/support",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "0.00000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_forward_active_energy",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "0.00000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_reverse_active_energy",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "0.00000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_forward_active_energy_balanced",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, "0.00000", false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_reverse_active_energy_balanced",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.0", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 1));

  //------------

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.0", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 2));

  //------------

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.00000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.000", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "0.0", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 3));

  ASSERT_FALSE(dataExists(&provider));
}

TEST_F(MqttStateMessageProviderTest, electricityMeterWithoutMeasurements) {
  SetResultValue(SUPLA_CHANNELFNC_ELECTRICITY_METER, true,
                 new supla_channel_value());

  SetElectricityMeasurementResult(EM_VAR_ALL, 0);

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, "true", false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/connected",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/total_cost",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/total_cost_balanced",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/price_per_unit",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/currency",
                              789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/support",
                              789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, NULL, false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_forward_active_energy",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, NULL, false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_reverse_active_energy",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, NULL, false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_forward_active_energy_balanced",
      789));

  ASSERT_TRUE(fetchAndCompare(
      &provider, NULL, NULL, false,
      "supla/9920767494dd87196e1896c7cbab707c/devices/456/channels/%i/state/"
      "total_reverse_active_energy_balanced",
      789));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 1));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 1));

  //------------

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 2));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 2));

  //------------

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_forward_active_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/"
                              "%i/total_reverse_active_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_forward_reactive_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "total_reverse_reactive_energy",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "frequency",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "voltage",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "current",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_active",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_reactive",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_apparent",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "power_factor",
                              789, 3));

  ASSERT_TRUE(fetchAndCompare(&provider, NULL, NULL, false,
                              "supla/9920767494dd87196e1896c7cbab707c/devices/"
                              "456/channels/%i/state/phases/%i/"
                              "phase_angle",
                              789, 3));

  ASSERT_FALSE(dataExists(&provider));
}

} /* namespace testing */
