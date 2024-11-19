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

#include <map>
#include <string>
#include <vector>

#include "device/channel_state.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_valve_value.h"
#include "jsonconfig/json_config.h"
#include "vbt/vbt_on_change_condition.h"

using std::map;
using std::string;
using std::vector;

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

TEST_F(OnChangeConditionTest, onChangeTo_allPredictedVarNames) {
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

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"fae1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae1);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"fae2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae2);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"fae3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae3);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"fae_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae_sum);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"fae_balanced\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae_balanced);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"rae1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae1);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"rae2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae2);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"rae3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae3);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"rae_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae_sum);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"rae_balanced\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae_balanced);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"counter\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_counter);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"calculated_value\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calculated_value);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"heating\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_heating);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"cooling\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_cooling);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"is_on\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_is_on);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"is_any_error_set\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_is_any_error_set);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"calibration_failed\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calibration_failed);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"calibration_lost\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calibration_lost);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"motor_problem\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_motor_problem);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"calibration_in_progress\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calibration_in_progress);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":100,\"name\":\"battery_level\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_battery_level);

  json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"battery_powered\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_battery_powered);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"is_battery_cover_open\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_is_battery_cover_open);

  json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":1,\"name\":\"thermometer_error\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_thermometer_error);

  json = cJSON_Parse("{\"on_change_to\":{\"eq\":1,\"name\":\"clock_error\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_clock_error);
}

TEST_F(OnChangeConditionTest, onChange_allPredictedVarNames) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{\"name\":\"color\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_color);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"color_brightness\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_color_brightness);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"brightness\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_brightness);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"temperature\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_temperature);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"humidity\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_humidity);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"flooding\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_flooding);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"manually_closed\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_manually_closed);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"voltage_avg\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage_avg);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"voltage1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage1);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"voltage2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage2);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"voltage3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_voltage3);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"current_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current_sum);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"current1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current1);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"current2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current2);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"current3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_current3);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_active_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active_sum);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_active1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active1);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_active2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active2);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_active3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_active3);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_reactive_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive_sum);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_reactive1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive1);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_reactive2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive2);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_reactive3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_reactive3);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_apparent_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent_sum);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_apparent1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent1);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_apparent2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent2);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"power_apparent3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_power_apparent3);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"fae1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae1);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"fae2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae2);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"fae3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae3);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"fae_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae_sum);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"fae_balanced\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_fae_balanced);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"rae1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae1);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"rae2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae2);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"rae3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae3);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"rae_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae_sum);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"rae_balanced\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_rae_balanced);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"counter\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_counter);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"calculated_value\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calculated_value);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"heating\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_heating);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"cooling\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_cooling);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"is_on\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_is_on);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"is_any_error_set\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_is_any_error_set);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"calibration_failed\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calibration_failed);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"calibration_lost\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calibration_lost);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"motor_problem\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_motor_problem);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"calibration_in_progress\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_calibration_in_progress);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"battery_powered\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_battery_powered);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"battery_level\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_battery_level);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"is_battery_cover_open\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_is_battery_cover_open);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"thermometer_error\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_thermometer_error);

  json = cJSON_Parse("{\"on_change\":{\"name\":\"clock_error\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_EQ(c.get_var_name(), var_name_clock_error);
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
  oldv.set_temperature(22.0);
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
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].voltage[0] = 24000;
  v3b.m[0].voltage[1] = 23000;
  v3b.m[0].voltage[2] = 22000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":230,\"name\":\"voltage_avg\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, voltage1) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].voltage[0] = 24000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":240,\"name\":\"voltage1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, voltage2) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].voltage[1] = 23000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":230,\"name\":\"voltage2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, voltage3) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].voltage[2] = 22000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":220,\"name\":\"voltage3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, currentSum) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].current[0] = 10000;
  v3b.m[0].current[1] = 20000;
  v3b.m[0].current[2] = 30000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":60,\"name\":\"current_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, current1) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].current[0] = 10000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":10,\"name\":\"current1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, current2) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].current[1] = 20000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":20,\"name\":\"current2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, current3) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].current[2] = 30000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":30,\"name\":\"current3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActiveSum) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_active[0] = 1000000;
  v3b.m[0].power_active[1] = 2000000;
  v3b.m[0].power_active[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":60,\"name\":\"power_active_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActive1) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_active[0] = 1000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":10,\"name\":\"power_active1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActive2) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_active[1] = 2000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":20,\"name\":\"power_active2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerActive3) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_active[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":30,\"name\":\"power_active3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerRectiveSum) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_reactive[0] = 1000000;
  v3b.m[0].power_reactive[1] = 2000000;
  v3b.m[0].power_reactive[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":60,\"name\":\"power_reactive_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerRective1) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_reactive[0] = 1000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":10,\"name\":\"power_reactive1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerReactive2) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_reactive[1] = 2000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":20,\"name\":\"power_reactive2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerRective3) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_reactive[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":30,\"name\":\"power_reactive3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparentSum) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_apparent[0] = 1000000;
  v3b.m[0].power_apparent[1] = 2000000;
  v3b.m[0].power_apparent[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":60,\"name\":\"power_apparent_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparent1) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_apparent[0] = 1000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":10,\"name\":\"power_apparent1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparent2) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_apparent[1] = 2000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":20,\"name\":\"power_apparent2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, powerApparent3) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};
  v3b.m_count = 1;
  v3b.m[0].power_apparent[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":30,\"name\":\"power_apparent3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, forwardActiveEnergy1) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_forward_active_energy[0] = 1000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":10,\"name\":\"fae1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, forwardActiveEnergy2) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_forward_active_energy[1] = 2000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":20,\"name\":\"fae2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, forwardActiveEnergy3) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_forward_active_energy[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":30,\"name\":\"fae3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, forwardActiveEnergySum) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_forward_active_energy[0] = 1000000;
  v3b.total_forward_active_energy[1] = 2000000;
  v3b.total_forward_active_energy[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":60,\"name\":\"fae_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, forwardActiveEnergyBalanced) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_forward_active_energy_balanced = 3300000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":33,\"name\":\"fae_balanced\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, reverseActiveEnergy1) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_reverse_active_energy[0] = 1000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":10,\"name\":\"rae1\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, reverseActiveEnergy2) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_reverse_active_energy[1] = 2000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":20,\"name\":\"rae2\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, reverseActiveEnergy3) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_reverse_active_energy[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change_to\":{\"eq\":30,\"name\":\"rae3\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, reverseActiveEnergySum) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_reverse_active_energy[0] = 1000000;
  v3b.total_reverse_active_energy[1] = 2000000;
  v3b.total_reverse_active_energy[2] = 3000000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":60,\"name\":\"rae_sum\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, reversectiveEnergyBalanced) {
  TElectricityMeter_ExtendedValue_V3 v3a = {};
  TElectricityMeter_ExtendedValue_V3 v3b = {};

  v3b.total_reverse_active_energy_balanced = 3300000;

  supla_channel_em_extended_value oldv(&v3a, nullptr, 0);
  supla_channel_em_extended_value newv(&v3b, nullptr, 0);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":33,\"name\":\"rae_balanced\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, icCounter) {
  TDS_ImpulseCounter_Value ic_val1 = {};
  TDS_ImpulseCounter_Value ic_val2 = {};
  ic_val1.counter = 151;

  supla_json_config cfg;
  cfg.set_user_config(
      "{\"currency\":\"PLN\",\"unit\":\"Unit\",\"pricePerUnit\":45000,"
      "\"impulsesPerUnit\":10}");

  supla_channel_ic_extended_value newv(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                       &ic_val1, &cfg);

  supla_channel_ic_extended_value oldv(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                       &ic_val2, &cfg);

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change_to\":{\"eq\":151,\"name\":\"counter\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, icCalculatedValue) {
  TDS_ImpulseCounter_Value ic_val1 = {};
  TDS_ImpulseCounter_Value ic_val2 = {};
  ic_val1.counter = 151;

  supla_json_config cfg;
  cfg.set_user_config(
      "{\"currency\":\"PLN\",\"unit\":\"Unit\",\"pricePerUnit\":45000,"
      "\"impulsesPerUnit\":10}");

  supla_channel_ic_extended_value newv(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                       &ic_val1, &cfg);

  supla_channel_ic_extended_value oldv(SUPLA_CHANNELFNC_IC_ELECTRICITY_METER,
                                       &ic_val2, &cfg);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse(
      "{\"on_change_to\":{\"eq\":15.1,\"name\":\"calculated_value\"}}");
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

TEST_F(OnChangeConditionTest, floatingPointValueChanged) {
  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  supla_channel_floating_point_sensor_value oldv, newv;
  oldv.set_value(56);
  newv.set_value(56);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_value(2);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  newv.set_value(1);
  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, temperatureChanged) {
  supla_channel_temphum_value oldv, newv;
  oldv.set_temperature(22.5);
  newv.set_temperature(22.5);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{\"name\":\"temperature\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_temperature(22.6);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  newv.set_temperature(22.7);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, heatingChanged) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue *)raw_value)->IsOn = 1;
  ((THVACValue *)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_HEATING;

  supla_channel_hvac_value oldv, newv;

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{\"name\":\"heating\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  newv.set_raw_value(raw_value);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  ((THVACValue *)raw_value)->IsOn = 0;

  newv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, coolingChanged) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue *)raw_value)->IsOn = 1;
  ((THVACValue *)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_COOLING;

  supla_channel_hvac_value oldv, newv;

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{\"name\":\"cooling\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  newv.set_raw_value(raw_value);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  ((THVACValue *)raw_value)->IsOn = 0;

  newv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, heatingOrCoolingChanged) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue *)raw_value)->IsOn = 1;
  ((THVACValue *)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_COOLING;

  supla_channel_hvac_value oldv, newv;

  supla_vbt_on_change_condition c;

  cJSON *json =
      cJSON_Parse("{\"on_change\":{\"name\":\"heating_or_cooling\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  ((THVACValue *)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_HEATING;
  oldv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  ((THVACValue *)raw_value)->Flags = SUPLA_HVAC_VALUE_FLAG_COOLING;
  newv.set_raw_value(raw_value);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  ((THVACValue *)raw_value)->IsOn = 0;

  newv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, isOnChanged) {
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((THVACValue *)raw_value)->IsOn = 1;

  supla_channel_hvac_value oldv, newv;

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{\"name\":\"is_on\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  oldv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

  newv.set_raw_value(raw_value);

  EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

  ((THVACValue *)raw_value)->IsOn = 0;

  newv.set_raw_value(raw_value);

  EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
}

TEST_F(OnChangeConditionTest, anyErrorChanged_hvac) {
  const vector<unsigned _supla_int16_t> flags = {
      SUPLA_HVAC_VALUE_FLAG_THERMOMETER_ERROR,
      SUPLA_HVAC_VALUE_FLAG_CLOCK_ERROR,
      SUPLA_HVAC_VALUE_FLAG_BATTERY_COVER_OPEN};

  for (auto it = flags.cbegin(); it != flags.cend(); ++it) {
    char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
    ((THVACValue *)raw_value)->Flags = *it;

    supla_channel_hvac_value oldv, newv;

    supla_vbt_on_change_condition c;

    cJSON *json =
        cJSON_Parse("{\"on_change\":{\"name\":\"is_any_error_set\"}}");
    c.apply_json_config(json);
    cJSON_Delete(json);

    EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

    oldv.set_raw_value(raw_value);

    EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

    newv.set_raw_value(raw_value);

    EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

    ((THVACValue *)raw_value)->Flags = 0;

    newv.set_raw_value(raw_value);

    EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
  }
}

TEST_F(OnChangeConditionTest, rollerShutterErrors) {
  const map<_supla_int16_t, string> flag_map = {
      {RS_VALUE_FLAG_CALIBRATION_FAILED, "calibration_failed"},
      {RS_VALUE_FLAG_CALIBRATION_LOST, "calibration_lost"},
      {RS_VALUE_FLAG_MOTOR_PROBLEM, "motor_problem"}};

  for (auto it = flag_map.cbegin(); it != flag_map.cend(); ++it) {
    char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
    supla_channel_rs_value oldv(raw_value);
    supla_channel_rs_value newv(raw_value);

    ((TDSC_RollerShutterValue *)raw_value)->flags = it->first;

    supla_vbt_on_change_condition c;

    cJSON *json =
        cJSON_Parse("{\"on_change\":{\"name\":\"is_any_error_set\"}}");
    c.apply_json_config(json);
    cJSON_Delete(json);

    EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

    oldv.set_raw_value(raw_value);

    EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

    newv.set_raw_value(raw_value);

    EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

    ((TDSC_RollerShutterValue *)raw_value)->flags = 0;

    newv.set_raw_value(raw_value);

    EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

    oldv.set_raw_value(raw_value);

    string json_str = "{\"on_change\":{\"name\":\"";
    json_str.append(it->second);
    json_str.append("\"}}");

    json = cJSON_Parse(json_str.c_str());
    c.apply_json_config(json);
    cJSON_Delete(json);

    EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

    ((TDSC_RollerShutterValue *)raw_value)->flags = it->first;

    oldv.set_raw_value(raw_value);

    EXPECT_TRUE(c.is_condition_met(&oldv, &newv));

    newv.set_raw_value(raw_value);

    EXPECT_FALSE(c.is_condition_met(&oldv, &newv));

    ((TDSC_RollerShutterValue *)raw_value)->flags = 0;

    newv.set_raw_value(raw_value);

    EXPECT_TRUE(c.is_condition_met(&oldv, &newv));
  }
}

TEST_F(OnChangeConditionTest, batteryLevelChanged) {
  TDSC_ChannelState raw = {};
  supla_channel_state olds(&raw);

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL;
  raw.BatteryLevel = 5;

  supla_channel_state news(&raw);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{\"name\":\"battery_level\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c.is_condition_met(&olds, &news));

  olds = news;

  EXPECT_FALSE(c.is_condition_met(&olds, &news));

  raw.BatteryLevel = 6;
  news = supla_channel_state(&raw);

  EXPECT_TRUE(c.is_condition_met(&olds, &news));
}

TEST_F(OnChangeConditionTest, batteryPoweredStateChanged) {
  TDSC_ChannelState raw = {};
  supla_channel_state olds(&raw);

  raw.Fields = SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED;
  raw.BatteryPowered = 1;

  supla_channel_state news(&raw);

  supla_vbt_on_change_condition c;

  cJSON *json = cJSON_Parse("{\"on_change\":{\"name\":\"battery_powered\"}}");
  c.apply_json_config(json);
  cJSON_Delete(json);

  EXPECT_FALSE(c.is_condition_met(&olds, &news));

  olds = news;

  EXPECT_FALSE(c.is_condition_met(&olds, &news));

  raw.BatteryPowered = 0;
  news = supla_channel_state(&raw);

  EXPECT_TRUE(c.is_condition_met(&olds, &news));
}

}  // namespace testing
