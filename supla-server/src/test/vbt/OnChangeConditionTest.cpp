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

#include "OnChangeConditionTest.h"

#include "device/extended_value/channel_em_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_valve_value.h"
#include "vbt/vbt_on_change_condition.h"

namespace testing {

OnChangeConditionTest::OnChangeConditionTest() : Test() {}

OnChangeConditionTest::~OnChangeConditionTest() {}

TEST_F(OnChangeConditionTest, defaults) {
  supla_vbt_on_change_condition c;
  EXPECT_EQ(c.get_value(), 0);
  EXPECT_EQ(c.get_var_name(), var_name_none);
  EXPECT_EQ(c.get_op(), op_unknown);
  EXPECT_EQ(c.get_resume_op(), op_unknown);
  EXPECT_EQ(c.get_resume_value(), 0);
  EXPECT_FALSE(c.is_paused());
}

TEST_F(OnChangeConditionTest, assignAllParams) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":123,\"name\":\"color\",\"resume\":{\"ne\":1}}"
      "}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_value(), 123);
  EXPECT_EQ(c.get_var_name(), var_name_color);
  EXPECT_EQ(c.get_op(), op_eq);
  EXPECT_EQ(c.get_resume_op(), op_ne);
  EXPECT_EQ(c.get_resume_value(), 1);
}

TEST_F(OnChangeConditionTest, allPredictedOperators) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":1}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_eq);

  json = cJSON_Parse("{\"on_change_to\":{\"ne\":1}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_ne);

  json = cJSON_Parse("{\"on_change_to\":{\"le\":1}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_le);

  json = cJSON_Parse("{\"on_change_to\":{\"lt\":1}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_lt);

  json = cJSON_Parse("{\"on_change_to\":{\"ge\":1}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_ge);

  json = cJSON_Parse("{\"on_change_to\":{\"gt\":1}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_gt);
}

TEST_F(OnChangeConditionTest, allPredictedVarNames) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"color\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_color);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"color_brightness\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_color_brightness);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"brightness\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_brightness);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"temperature\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_temperature);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"humidity\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_humidity);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"flooding\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_flooding);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"manually_closed\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_manually_closed);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"voltage_avg\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage_avg);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"voltage1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage1);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"voltage2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage2);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"voltage3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage3);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"current_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current_sum);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"current1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current1);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"current2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current2);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"current3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current3);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"power_active_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active_sum);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_active1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active1);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_active2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active2);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_active3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active3);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"power_reactive_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive_sum);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_reactive1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive1);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_reactive2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive2);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_reactive3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive3);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"power_apparent_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent_sum);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_apparent1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent1);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_apparent2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent2);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"power_apparent3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent3);
}

TEST_F(OnChangeConditionTest, boolValues) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":true}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_value(), 1);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":\"hi\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_value(), 1);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":\"on\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_value(), 1);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":false}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(),
            op_eq);  // 0 may be a parse error, so we also check the operator
  EXPECT_EQ(c.get_value(), 0);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":\"lo\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_eq);
  EXPECT_EQ(c.get_value(), 0);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":\"low\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_eq);
  EXPECT_EQ(c.get_value(), 0);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":\"off\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_op(), op_eq);
  EXPECT_EQ(c.get_value(), 0);
}

TEST_F(OnChangeConditionTest, intValue) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":123}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_value(), 123);
}

TEST_F(OnChangeConditionTest, doubleValue) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":123.456}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_value(), 123.456);
}

TEST_F(OnChangeConditionTest, equalityOperator) {
  supla_vbt_on_change_condition c1;
  supla_vbt_on_change_condition c2;

  EXPECT_TRUE(c1 == c2);
  cJSON *base_json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":123,\"name\":\"color\",\"resume\":{\"eq\":1}}"
      "}");
  c1.apply_json_config(base_json);
  c2.apply_json_config(base_json);

  EXPECT_EQ(c1.get_op(),
            op_eq);  // We confirm that json is ok through read operator.

  EXPECT_TRUE(c1 == c2);

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"ne\":123,\"name\":\"color\",\"resume\":{\"eq\":1}}"
      "}");
  c1.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c1 == c2);
  c1.apply_json_config(base_json);
  EXPECT_TRUE(c1 == c2);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":12,\"name\":\"color\",\"resume\":{"
      "\"eq\":1}}"
      "}");
  c1.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c1 == c2);
  c1.apply_json_config(base_json);
  EXPECT_TRUE(c1 == c2);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":123,\"name\":\"brightness\",\"resume\":{"
      "\"eq\":1}}"
      "}");
  c1.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c1 == c2);
  c1.apply_json_config(base_json);
  EXPECT_TRUE(c1 == c2);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":123,\"name\":\"color\",\"resume\":{\"gt\":1}}"
      "}");
  c1.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c1 == c2);
  c1.apply_json_config(base_json);
  EXPECT_TRUE(c1 == c2);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":123,\"name\":\"color\",\"resume\":{\"eq\":2}}"
      "}");
  c1.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c1 == c2);
  c1.apply_json_config(base_json);
  EXPECT_TRUE(c1 == c2);

  cJSON_Delete(base_json);
}

TEST_F(OnChangeConditionTest, valueComparsion_Eq) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":123.456}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  supla_channel_floating_point_sensor_value oldv, newv;
  oldv.set_value(123.456);
  newv.set_value(123.456);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(2);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  newv.set_value(123);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, valueComparsion_Ne) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"ne\":55}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  supla_channel_floating_point_sensor_value oldv, newv;
  oldv.set_value(56);
  newv.set_value(56);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(2);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  newv.set_value(55);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, valueComparsion_Gt) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"gt\":100}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  supla_channel_floating_point_sensor_value oldv, newv;
  oldv.set_value(101);
  newv.set_value(101);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(102);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(100);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, valueComparsion_Ge) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"ge\":100}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  supla_channel_floating_point_sensor_value oldv, newv;
  oldv.set_value(100);
  newv.set_value(100);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(102);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(99);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(101);
  newv.set_value(101);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(102);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(99);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, valueComparsion_Lt) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"lt\":100}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  supla_channel_floating_point_sensor_value oldv, newv;
  oldv.set_value(99);
  newv.set_value(99);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(98);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(100);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, valueComparsion_Le) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"le\":100}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  supla_channel_floating_point_sensor_value oldv, newv;
  oldv.set_value(100);
  newv.set_value(100);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(99);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(101);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(100);
  newv.set_value(100);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(99);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(101);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, binarySensor) {
  supla_channel_binary_sensor_value oldv, newv;
  oldv.set_hi(false);
  newv.set_hi(true);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":true}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, onoffValue) {
  supla_channel_onoff_value oldv, newv;
  oldv.set_on(true);
  newv.set_on(false);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":false}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, color) {
  supla_channel_rgbw_value oldv, newv;
  newv.set_color(1234);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1234,\"name\":\"color\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, colorBrightness) {
  supla_channel_rgbw_value oldv, newv;
  newv.set_color_brightness(78);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":78,\"name\":\"color_brightness\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, brightness) {
  supla_channel_rgbw_value oldv, newv;
  newv.set_brightness(5);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":5,\"name\":\"brightness\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, temperature) {
  supla_channel_temphum_value oldv, newv;
  newv.set_temperature(22.5);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":22.5,\"name\":\"temperature\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, humidity) {
  supla_channel_temphum_value oldv(true, 0.0, 0.0);
  supla_channel_temphum_value newv(true, 0.0, 45.0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":45,\"name\":\"humidity\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, avgVoltage) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].voltage[0] = 24000;
  v2b.m[0].voltage[1] = 23000;
  v2b.m[0].voltage[2] = 22000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":230,\"name\":\"voltage_avg\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, voltage1) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].voltage[0] = 24000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":240,\"name\":\"voltage1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, voltage2) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].voltage[1] = 23000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":230,\"name\":\"voltage2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, voltage3) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].voltage[2] = 22000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":220,\"name\":\"voltage3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, currentSum) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].current[0] = 10000;
  v2b.m[0].current[1] = 20000;
  v2b.m[0].current[2] = 30000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":60,\"name\":\"current_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, current1) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].current[0] = 10000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":10,\"name\":\"current1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, current2) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].current[1] = 20000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":20,\"name\":\"current2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, current3) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].current[2] = 30000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":30,\"name\":\"current3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActiveSum) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_active[0] = 1000000;
  v2b.m[0].power_active[1] = 2000000;
  v2b.m[0].power_active[2] = 3000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":60,\"name\":\"power_active_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActive1) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_active[0] = 1000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":10,\"name\":\"power_active1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActive2) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_active[1] = 2000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":20,\"name\":\"power_active2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActive3) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_active[2] = 3000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":30,\"name\":\"power_active3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerRectiveSum) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_reactive[0] = 1000000;
  v2b.m[0].power_reactive[1] = 2000000;
  v2b.m[0].power_reactive[2] = 3000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":60,\"name\":\"power_reactive_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerRective1) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_reactive[0] = 1000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":10,\"name\":\"power_reactive1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerReactive2) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_reactive[1] = 2000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":20,\"name\":\"power_reactive2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerRective3) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_reactive[2] = 3000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":30,\"name\":\"power_reactive3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparentSum) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_apparent[0] = 1000000;
  v2b.m[0].power_apparent[1] = 2000000;
  v2b.m[0].power_apparent[2] = 3000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":60,\"name\":\"power_apparent_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparent1) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_apparent[0] = 1000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":10,\"name\":\"power_apparent1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparent2) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_apparent[1] = 2000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":20,\"name\":\"power_apparent2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparent3) {
  TElectricityMeter_ExtendedValue_V2 v2a = {};
  TElectricityMeter_ExtendedValue_V2 v2b = {};
  v2b.m_count = 1;
  v2b.m[0].power_apparent[2] = 3000000;

  supla_channel_em_extended_value oldv(&v2a, nullptr, 0);
  supla_channel_em_extended_value newv(&v2b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":30,\"name\":\"power_apparent3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, withoutPausing) {
  supla_channel_temphum_value oldv, newv;
  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"lt\":20,\"name\":\"temperature\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  oldv.set_temperature(23);
  newv.set_temperature(19);

  for (int a = 0; a < 10; a++) {
    EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
    EXPECT_FALSE(c.is_paused());
  }
}

TEST_F(OnChangeConditionTest, resume) {
  supla_channel_temphum_value oldv, newv;
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"lt\":20,\"resume\":{\"ge\":22},\"name\":"
      "\"temperature\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  oldv.set_temperature(23);
  newv.set_temperature(19);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
  EXPECT_TRUE(c.is_paused());

  oldv.set_temperature(19);
  newv.set_temperature(23);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));
  EXPECT_FALSE(c.is_paused());

  oldv.set_temperature(23);
  newv.set_temperature(19);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
  EXPECT_TRUE(c.is_paused());
}

TEST_F(OnChangeConditionTest, resume_AnotherCase) {
  supla_channel_temphum_value oldv, newv;
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"lt\":20,\"resume\":{\"ge\":22},\"name\":"
      "\"temperature\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  oldv.set_temperature(23);
  newv.set_temperature(19);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
  EXPECT_TRUE(c.is_paused());

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));
  EXPECT_TRUE(c.is_paused());

  oldv.set_temperature(19);
  newv.set_temperature(23);
  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));
  EXPECT_FALSE(c.is_paused());

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));
  EXPECT_FALSE(c.is_paused());

  oldv.set_temperature(23);
  newv.set_temperature(19);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
  EXPECT_TRUE(c.is_paused());
}

}  // namespace testing
