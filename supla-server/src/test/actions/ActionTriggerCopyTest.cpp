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

#include "proto.h"

namespace testing {

void ActionTriggerCopyTest::SetUp() {
  ActionTriggerTest::SetUp();
  at_config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channelGroup\",\"subjectId\":31,\"action\":{\"id\":"
      "10100,\"param\":{\"sourceChannelId\":46868}}}}}");

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

TEST_F(ActionTriggerCopyTest, rollerShutter) {
  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 0);

  value_getter->setResult(true);

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 0);

  value_getter->setFunction(SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER);

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 1);
  EXPECT_EQ(aexec->getClosingPercentage(), 0);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 31);
  EXPECT_EQ(value_getter->get_device_id(), 0);
  EXPECT_EQ(value_getter->get_channel_id(), 46868);
  EXPECT_EQ(value_getter->get_user_id(), 1);

  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  value[0] = -1;
  value_getter->setValue(value);

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 1);

  value[0] = 45;
  value_getter->setValue(value);

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 2);
  EXPECT_EQ(aexec->getClosingPercentage(), 45);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 31);

  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectType\":\"channel\",\"subjectId\":"
      "455,\"action\":{\"id\":10100,\"param\":{\"sourceChannelId\":68}}}}}");

  value[0] = 80;
  value_getter->setValue(value);

  at->execute_actions(1, 0, SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(value_getter->get_channel_id(), 68);
  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 3);
  EXPECT_EQ(aexec->getClosingPercentage(), 80);
  EXPECT_EQ(aexec->get_channel_id(), 455);
  EXPECT_EQ(aexec->get_group_id(), 0);
}

} /* namespace testing */
