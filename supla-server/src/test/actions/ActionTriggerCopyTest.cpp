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

#include "ActionTriggerCopyTest.h"

#include "device/channel_gate_value.h"
#include "device/channel_onoff_value.h"
#include "device/channel_rgbw_value.h"
#include "device/channel_rs_value.h"
#include "proto.h"

namespace testing {

void ActionTriggerCopyTest::SetUp() {
  ActionTriggerTest::SetUp();
  at_config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channelGroup\",\"subjectId\":31,\"action\":{\"id\":"
      "10100,\"param\":{\"sourceDeviceId\":66,"
      "\"sourceChannelId\":46868}}}}}");

  at_config->set_properties(
      "{\"actionTriggerCapabilities\":[\"TURN_ON\",\"TURN_OFF\",\"TOGGLE_X1\","
      "\"TOGGLE_X2\",\"TOGGLE_X3\",\"TOGGLE_X4\",\"TOGGLE_X5\"]}");

  supla_user::find(1, true);
}

void ActionTriggerCopyTest::TearDown() {
  ActionTriggerTest::TearDown();

  supla_user *user = supla_user::find(1, false);
  if (user) {
    delete user;
  }
}

TEST_F(ActionTriggerCopyTest, empty) {
  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 0);
}

TEST_F(ActionTriggerCopyTest, gate) {
  value_getter->setResult(
      new supla_channel_gate_value(gsl_unknown, gsl_unknown));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 0);

  value_getter->setResult(
      new supla_channel_gate_value(gsl_unknown, gsl_closed));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 0);

  value_getter->setResult(
      new supla_channel_gate_value(gsl_closed, gsl_unknown));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);

  EXPECT_EQ(aexec->getCloseCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 31);
  EXPECT_EQ(value_getter->get_device_id(), 66);
  EXPECT_EQ(value_getter->get_channel_id(), 46868);
  EXPECT_EQ(value_getter->get_user_id(), 1);

  value_getter->setResult(new supla_channel_gate_value(gsl_open, gsl_unknown));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 2);
  EXPECT_EQ(aexec->getOpenCounter(), 1);
}

TEST_F(ActionTriggerCopyTest, rollerShutter) {
  TDSC_RollerShutterValue rsval = {};
  value_getter->setResult(new supla_channel_rs_value(&rsval));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 1);
  EXPECT_EQ(aexec->getClosingPercentage(), 0);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 31);
  EXPECT_EQ(value_getter->get_device_id(), 66);
  EXPECT_EQ(value_getter->get_channel_id(), 46868);
  EXPECT_EQ(value_getter->get_user_id(), 1);

  rsval.position = -1;
  value_getter->setResult(new supla_channel_rs_value(&rsval));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 1);

  rsval.position = 45;
  value_getter->setResult(new supla_channel_rs_value(&rsval));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 2);
  EXPECT_EQ(aexec->getClosingPercentage(), 45);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 31);

  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectType\":\"channel\",\"subjectId\":"
      "455,\"action\":{\"id\":10100,\"param\":{\"sourceChannelId\":68}}}}}");

  rsval.position = 80;
  value_getter->setResult(new supla_channel_rs_value(&rsval));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(value_getter->get_channel_id(), 68);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 3);
  EXPECT_EQ(aexec->getClosingPercentage(), 80);
  EXPECT_EQ(value_getter->get_device_id(), 0);
  EXPECT_EQ(aexec->get_channel_id(), 455);
  EXPECT_EQ(aexec->get_group_id(), 0);
}

TEST_F(ActionTriggerCopyTest, onOff) {
  value_getter->setResult(new supla_channel_onoff_value(true));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getOnCounter(), 1);
  EXPECT_EQ(aexec->getOffCounter(), 0);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 31);
  EXPECT_EQ(value_getter->get_device_id(), 66);
  EXPECT_EQ(value_getter->get_channel_id(), 46868);
  EXPECT_EQ(value_getter->get_user_id(), 1);

  value_getter->setResult(new supla_channel_onoff_value(false));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 2);
  EXPECT_EQ(aexec->getOffCounter(), 1);
}

TEST_F(ActionTriggerCopyTest, rgbw) {
  TRGBW_Value rgbw = {};
  value_getter->setResult(new supla_channel_rgbw_value(&rgbw));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getRGBWCounter(), 1);
  EXPECT_EQ(aexec->getColor(), (unsigned int)0);
  EXPECT_EQ(aexec->getColorBrightness(), 0);
  EXPECT_EQ(aexec->getBrightness(), 0);
  EXPECT_EQ(aexec->getRGBWOnOff(), RGBW_BRIGHTNESS_ONOFF | RGBW_COLOR_ONOFF);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 31);
  EXPECT_EQ(value_getter->get_device_id(), 66);
  EXPECT_EQ(value_getter->get_channel_id(), 46868);
  EXPECT_EQ(value_getter->get_user_id(), 1);

  rgbw.R = 0xAA;
  rgbw.G = 0xBB;
  rgbw.B = 0xCC;
  rgbw.colorBrightness = 50;

  value_getter->setResult(new supla_channel_rgbw_value(&rgbw));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getRGBWCounter(), 2);
  EXPECT_EQ(aexec->getColor(), (unsigned int)0xAABBCC);
  EXPECT_EQ(aexec->getColorBrightness(), 50);
  EXPECT_EQ(aexec->getBrightness(), 0);
  EXPECT_EQ(aexec->getRGBWOnOff(), RGBW_BRIGHTNESS_ONOFF);

  rgbw.R = 0xA1;
  rgbw.G = 0xB2;
  rgbw.B = 0xC3;
  rgbw.colorBrightness = 0;
  rgbw.brightness = 15;

  value_getter->setResult(new supla_channel_rgbw_value(&rgbw));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getRGBWCounter(), 3);
  EXPECT_EQ(aexec->getColor(), (unsigned int)0xA1B2C3);
  EXPECT_EQ(aexec->getColorBrightness(), 0);
  EXPECT_EQ(aexec->getBrightness(), 15);
  EXPECT_EQ(aexec->getRGBWOnOff(), RGBW_COLOR_ONOFF);

  rgbw.R = 0xA1;
  rgbw.G = 0x00;
  rgbw.B = 0xC3;
  rgbw.colorBrightness = 25;
  rgbw.brightness = 15;

  value_getter->setResult(new supla_channel_rgbw_value(&rgbw));

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getRGBWCounter(), 4);
  EXPECT_EQ(aexec->getColor(), (unsigned int)0xA100C3);
  EXPECT_EQ(aexec->getColorBrightness(), 25);
  EXPECT_EQ(aexec->getBrightness(), 15);
  EXPECT_EQ(aexec->getRGBWOnOff(), 0);
}

} /* namespace testing */
