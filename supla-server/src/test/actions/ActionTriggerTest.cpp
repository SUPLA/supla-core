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

#include "ActionTriggerTest.h"

#include "proto.h"

namespace testing {

ActionTriggerTest::ActionTriggerTest(void) {
  at = NULL;
  at_config = NULL;
  aexec = NULL;
  value_getter = NULL;
}
ActionTriggerTest::~ActionTriggerTest(void) {}

void ActionTriggerTest::SetUp() {
  at_config = new action_trigger_config(NULL);
  EXPECT_TRUE(at_config != NULL);

  aexec = new ActionExecutorMock();
  EXPECT_TRUE(aexec != NULL);

  value_getter = new ValueGetterMock();
  EXPECT_TRUE(value_getter != NULL);

  at = new supla_action_trigger(aexec, at_config, value_getter);
  EXPECT_TRUE(at != NULL);
}

void ActionTriggerTest::TearDown() {
  if (value_getter) {
    delete value_getter;
    value_getter = NULL;
  }

  if (at_config) {
    delete at_config;
    at_config = NULL;
  }

  if (aexec) {
    delete aexec;
    aexec = NULL;
  }

  if (at) {
    delete at;
    at = NULL;
  }
}

TEST_F(ActionTriggerTest, open) {
  at_config->set_user_config(
      "{\"actions\":{\"HOLD\":{\"subjectId\":3611,\"subjectType\":\"channel\","
      "\"action\":{\"id\":10,\"param\":[]}}}}");

  at->execute_actions(2, 1, SUPLA_ACTION_CAP_HOLD);
  EXPECT_EQ(aexec->counterSetCount(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctUnknown, 0));

  at_config->set_capabilities(SUPLA_ACTION_CAP_HOLD);

  at->execute_actions(331122, 1, SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getOpenCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 3611);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 331122));
}

TEST_F(ActionTriggerTest, openGroup) {
  at_config->set_user_config(
      "{\"actions\":{\"HOLD\":{\"subjectId\":3611,\"subjectType\":"
      "\"channelGroup\","
      "\"action\":{\"id\":10,\"param\":[]}}}}");

  at->execute_actions(5, 1, SUPLA_ACTION_CAP_HOLD);
  EXPECT_EQ(aexec->counterSetCount(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctUnknown, 0));

  at_config->set_capabilities(SUPLA_ACTION_CAP_HOLD);

  at->execute_actions(6, 1, SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getOpenCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 0);
  EXPECT_EQ(aexec->get_group_id(), 3611);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 6));
}

TEST_F(ActionTriggerTest, close) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":4611,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":20,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);
  at->execute_actions(33, 1, SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getCloseCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 4611);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 33));
}

TEST_F(ActionTriggerTest, shut) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X2\":{\"subjectId\":461,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":30,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2);
  at->execute_actions(44, 1, SUPLA_ACTION_CAP_TOGGLE_x2);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 461);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 44));
}

TEST_F(ActionTriggerTest, reveal) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X3\":{\"subjectId\":4,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":40,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x3);
  at->execute_actions(75, 1, SUPLA_ACTION_CAP_TOGGLE_x3);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getRevealCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 4);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 75));
}

TEST_F(ActionTriggerTest, revealPartially) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X4\":{\"subjectId\":4,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":50,\"param\":{\"percentage\":11}}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x4);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x4);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getShutCounter(), 1);
  EXPECT_EQ(aexec->getClosingPercentage(), 89);
  EXPECT_EQ(aexec->get_channel_id(), 4);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, upOrStop) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X2\":{\"subjectId\":4461,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":140,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x2);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getUpOrStopCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 4461);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, downOrStop) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X2\":{\"subjectId\":44611,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":150,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x2);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getDownOrStopCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 44611);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, stepByStep) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X2\":{\"subjectId\":446111,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":160,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x2);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getStepByStepCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 446111);
  EXPECT_EQ(aexec->get_group_id(), 0);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, turnOn) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X4\":{\"subjectId\":5,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":60,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x4);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x4);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getOnCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 5);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, turnOff) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X5\":{\"subjectId\":6,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":70,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x5);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x5);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getOffCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 6);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, setRGBWParameters) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X1\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":80,\"param\":{\"brightness\":12,\"hue\":35,\"color_"
      "brightness\":31}}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x1);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_SHORT_PRESS_x1);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getRGBWCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 6789);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, openClose) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X2\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":90}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x2);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_SHORT_PRESS_x2);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getOpenCloseCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 6789);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, stop) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X3\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":100}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x3);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_SHORT_PRESS_x3);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getStopCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 6789);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, toggle) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X4\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":110}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x4);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_SHORT_PRESS_x4);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getToggleCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 6789);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, multiAction) {
  at_config->set_user_config(
      "{\"actions\":{\"HOLD\":{\"subjectId\":3611,\"subjectType\":\"channel\","
      "\"action\":{\"id\":10,\"param\":[]}}, "
      "\"SHORT_PRESS_X1\":{\"subjectId\":6789,\"subjectType\":\"channel\","
      "\"action\":{\"id\":80,\"param\":{\"brightness\":12,\"hue\":35,\"color_"
      "brightness\":31}}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x1 |
                              SUPLA_ACTION_CAP_HOLD);
  at->execute_actions(5, 1,
                      SUPLA_ACTION_CAP_SHORT_PRESS_x1 | SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(aexec->counterSetCount(), 2);
  EXPECT_EQ(aexec->getOpenCounter(), 1);
  EXPECT_EQ(aexec->getRGBWCounter(), 1);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, forwardOutside) {
  at_config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"other\",\"action\":{\"id\":10000}},\"TOGGLE_X2\":{"
      "\"subjectType\":\"other\",\"action\":{\"id\":10000}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2);
  at_config->set_channel_id_if_subject_not_set(5);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x2);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getForwardOutsideCounter(), 1);
  EXPECT_EQ(aexec->get_channel_id(), 5);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, executeScene) {
  at_config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"HOLD\":{\"subjectType\":"
      "\"scene\",\"subjectId\":20,\"action\":{\"id\":3000,\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_HOLD);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getExecuteCounter(), 1);
  EXPECT_EQ(aexec->get_scene_id(), 20);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

TEST_F(ActionTriggerTest, interruptScene) {
  at_config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"scene\",\"subjectId\":20,\"action\":{\"id\":3001,"
      "\"param\":[]}}}}");

  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);
  at->execute_actions(5, 1, SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(aexec->counterSetCount(), 1);
  EXPECT_EQ(aexec->getInterruptCounter(), 1);
  EXPECT_EQ(aexec->get_scene_id(), 20);
  EXPECT_TRUE(aexec->get_caller() == supla_caller(ctActionTrigger, 5));
}

} /* namespace testing */
