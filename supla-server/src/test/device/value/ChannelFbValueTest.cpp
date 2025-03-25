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

#include "ChannelFbValueTest.h"

#include "device/value/channel_fb_value.h"
#include "jsonconfig/channel/facade_blind_config.h"

namespace testing {

TEST_F(ChannelFbValueTest, rawValueSetterAndGetter) {
  TDSC_FacadeBlindValue value = {};

  value.tilt = 1;
  value.flags = 2;
  value.position = 3;

  supla_channel_fb_value v1(&value);
  EXPECT_EQ(memcmp(v1.get_fb_value(), &value, sizeof(TDSC_FacadeBlindValue)),
            0);

  supla_channel_fb_value v2(v1.get_fb_value());

  EXPECT_EQ(memcmp(v2.get_fb_value(), v1.get_fb_value(),
                   sizeof(TDSC_RollerShutterValue)),
            0);

  value.position = 50;
  v2.set_fb_value(&value);
  value.position = 100;

  EXPECT_EQ(v2.get_fb_value()->position, 50);
}

TEST_F(ChannelFbValueTest, getPosition) {
  TDSC_FacadeBlindValue raw = {};
  raw.position = 55;

  supla_channel_fb_value value(&raw);
  EXPECT_EQ(value.get_position(), 55);
}

TEST_F(ChannelFbValueTest, getTilt) {
  TDSC_FacadeBlindValue raw = {};
  raw.tilt = 15;

  supla_channel_fb_value value(&raw);
  EXPECT_EQ(value.get_tilt(), 15);
}

TEST_F(ChannelFbValueTest, getTiltAngle) {
  TDSC_FacadeBlindValue raw = {};
  raw.tilt = 10;

  facade_blind_config cfg;
  cfg.set_user_config("{\"tilt0Angle\":5,\"tilt100Angle\":25}");

  supla_channel_fb_value value(&raw);
  EXPECT_EQ(value.get_tilt_angle(&cfg), 7);

  cfg.set_user_config("{\"tilt0Angle\":25,\"tilt100Angle\":5}");
  EXPECT_EQ(value.get_tilt_angle(&cfg), 23);
}

TEST_F(ChannelFbValueTest, getVbtValue) {
  TDSC_FacadeBlindValue raw = {};
  raw.position = 55;

  supla_channel_fb_value value(&raw);

  double vbt_value = 0;
  EXPECT_TRUE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 55);

  raw.position = 25;
  value = supla_channel_fb_value(&raw);

  EXPECT_TRUE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 25);

  EXPECT_TRUE(value.get_vbt_value(var_name_calibration_failed, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_calibration_lost, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_calibration_lost, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_motor_problem, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(
      value.get_vbt_value(var_name_calibration_in_progress, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  raw.flags = RS_VALUE_FLAG_CALIBRATION_FAILED;
  value = supla_channel_fb_value(&raw);

  EXPECT_TRUE(value.get_vbt_value(var_name_calibration_failed, &vbt_value));
  EXPECT_EQ(vbt_value, 1);
  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  raw.flags = RS_VALUE_FLAG_CALIBRATION_LOST;
  value = supla_channel_fb_value(&raw);

  EXPECT_TRUE(value.get_vbt_value(var_name_calibration_lost, &vbt_value));
  EXPECT_EQ(vbt_value, 1);
  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  raw.flags = RS_VALUE_FLAG_MOTOR_PROBLEM;
  value = supla_channel_fb_value(&raw);

  EXPECT_TRUE(value.get_vbt_value(var_name_motor_problem, &vbt_value));
  EXPECT_EQ(vbt_value, 1);
  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  raw.flags = RS_VALUE_FLAG_CALIBRATION_IN_PROGRESS;
  value = supla_channel_fb_value(&raw);

  EXPECT_TRUE(
      value.get_vbt_value(var_name_calibration_in_progress, &vbt_value));
  EXPECT_EQ(vbt_value, 1);
  EXPECT_TRUE(value.get_vbt_value(var_name_is_any_error_set, &vbt_value));
  EXPECT_EQ(vbt_value, 0);
}

}  // namespace testing
