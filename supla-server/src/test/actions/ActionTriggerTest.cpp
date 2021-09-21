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
  ca_exec = NULL;
  dev_finder = NULL;
}
ActionTriggerTest::~ActionTriggerTest(void) {}

void ActionTriggerTest::SetUp() {
  at_config = new action_trigger_config(NULL);
  EXPECT_TRUE(at_config != NULL);

  ca_exec = new ChannelActionExecutorMock();
  EXPECT_TRUE(ca_exec != NULL);

  dev_finder = new DeviceFinderStub();
  EXPECT_TRUE(dev_finder != NULL);

  at = new supla_action_trigger(ca_exec, at_config, dev_finder);
  EXPECT_TRUE(at != NULL);
}

void ActionTriggerTest::TearDown() {
  if (at_config) {
    delete at_config;
    at_config = NULL;
  }

  if (ca_exec) {
    delete ca_exec;
    ca_exec = NULL;
  }

  if (dev_finder) {
    delete dev_finder;
    dev_finder = NULL;
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

  dev_finder->setResult(123);
  at->execute_actions(1, SUPLA_ACTION_CAP_HOLD);
  EXPECT_EQ(ca_exec->counterSetCount(), 0);

  at_config->set_capabilities(SUPLA_ACTION_CAP_HOLD);

  at->execute_actions(1, SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getOpenCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 123);
  EXPECT_EQ(ca_exec->get_channel_id(), 3611);
  EXPECT_EQ(ca_exec->get_group_id(), 0);
}

TEST_F(ActionTriggerTest, openGroup) {
  at_config->set_user_config(
      "{\"actions\":{\"HOLD\":{\"subjectId\":3611,\"subjectType\":"
      "\"channelGroup\","
      "\"action\":{\"id\":10,\"param\":[]}}}}");

  dev_finder->setResult(123);
  at->execute_actions(1, SUPLA_ACTION_CAP_HOLD);
  EXPECT_EQ(ca_exec->counterSetCount(), 0);

  at_config->set_capabilities(SUPLA_ACTION_CAP_HOLD);

  at->execute_actions(1, SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getOpenCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 0);
  EXPECT_EQ(ca_exec->get_channel_id(), 0);
  EXPECT_EQ(ca_exec->get_group_id(), 3611);
}

TEST_F(ActionTriggerTest, close) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":4611,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":20,\"param\":[]}}}}");

  dev_finder->setResult(2);
  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);
  at->execute_actions(1, SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getCloseCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 2);
  EXPECT_EQ(ca_exec->get_channel_id(), 4611);
  EXPECT_EQ(ca_exec->get_group_id(), 0);
}

TEST_F(ActionTriggerTest, shut) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X2\":{\"subjectId\":461,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":30,\"param\":[]}}}}");

  dev_finder->setResult(3);
  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2);
  at->execute_actions(1, SUPLA_ACTION_CAP_TOGGLE_x2);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getShutCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 3);
  EXPECT_EQ(ca_exec->get_channel_id(), 461);
  EXPECT_EQ(ca_exec->get_group_id(), 0);
}

TEST_F(ActionTriggerTest, reveal) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X3\":{\"subjectId\":4,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":40,\"param\":[]}}}}");

  dev_finder->setResult(44);
  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x3);
  at->execute_actions(1, SUPLA_ACTION_CAP_TOGGLE_x3);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getRevealCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 44);
  EXPECT_EQ(ca_exec->get_channel_id(), 4);
}

TEST_F(ActionTriggerTest, revealPartially) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X4\":{\"subjectId\":4,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":50,\"param\":{\"percentage\":11}}}}}");

  dev_finder->setResult(44);
  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x4);
  at->execute_actions(1, SUPLA_ACTION_CAP_TOGGLE_x4);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getShutCounter(), 1);
  EXPECT_EQ(ca_exec->getClosingPercentage(), 89);
  EXPECT_EQ(ca_exec->get_device_id(), 44);
  EXPECT_EQ(ca_exec->get_channel_id(), 4);
}

TEST_F(ActionTriggerTest, turnOn) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X4\":{\"subjectId\":5,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":60,\"param\":[]}}}}");

  dev_finder->setResult(45);
  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x4);
  at->execute_actions(1, SUPLA_ACTION_CAP_TOGGLE_x4);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getOnCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 45);
  EXPECT_EQ(ca_exec->get_channel_id(), 5);
}

TEST_F(ActionTriggerTest, turnOff) {
  at_config->set_user_config(
      "{\"actions\":{\"TOGGLE_X5\":{\"subjectId\":6,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":70,\"param\":[]}}}}");

  dev_finder->setResult(499);
  at_config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x5);
  at->execute_actions(1, SUPLA_ACTION_CAP_TOGGLE_x5);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getOffCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 499);
  EXPECT_EQ(ca_exec->get_channel_id(), 6);
}

TEST_F(ActionTriggerTest, setRGBWParameters) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X1\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":80,\"param\":{\"brightness\":12,\"hue\":35,\"color_"
      "brightness\":31}}}}}");

  dev_finder->setResult(4999);
  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x1);
  at->execute_actions(1, SUPLA_ACTION_CAP_SHORT_PRESS_x1);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getRGBWCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 4999);
  EXPECT_EQ(ca_exec->get_channel_id(), 6789);
}

TEST_F(ActionTriggerTest, openClose) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X2\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":90}}}}");

  dev_finder->setResult(4999);
  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x2);
  at->execute_actions(1, SUPLA_ACTION_CAP_SHORT_PRESS_x2);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getOpenCloseCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 4999);
  EXPECT_EQ(ca_exec->get_channel_id(), 6789);
}

TEST_F(ActionTriggerTest, stop) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X3\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":100}}}}");

  dev_finder->setResult(4999);
  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x3);
  at->execute_actions(1, SUPLA_ACTION_CAP_SHORT_PRESS_x3);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getStopCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 4999);
  EXPECT_EQ(ca_exec->get_channel_id(), 6789);
}

TEST_F(ActionTriggerTest, toggle) {
  at_config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X4\":{\"subjectId\":6789,\"subjectType\":"
      "\"channel\","
      "\"action\":{\"id\":110}}}}");

  dev_finder->setResult(4999);
  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x4);
  at->execute_actions(1, SUPLA_ACTION_CAP_SHORT_PRESS_x4);

  EXPECT_EQ(ca_exec->counterSetCount(), 1);
  EXPECT_EQ(ca_exec->getToggleCounter(), 1);
  EXPECT_EQ(ca_exec->get_device_id(), 4999);
  EXPECT_EQ(ca_exec->get_channel_id(), 6789);
}

TEST_F(ActionTriggerTest, multiAction) {
  at_config->set_user_config(
      "{\"actions\":{\"HOLD\":{\"subjectId\":3611,\"subjectType\":\"channel\","
      "\"action\":{\"id\":10,\"param\":[]}}, "
      "\"SHORT_PRESS_X1\":{\"subjectId\":6789,\"subjectType\":\"channel\","
      "\"action\":{\"id\":80,\"param\":{\"brightness\":12,\"hue\":35,\"color_"
      "brightness\":31}}}}}");

  dev_finder->setResult(4999);
  at_config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x1 |
                              SUPLA_ACTION_CAP_HOLD);
  at->execute_actions(1,
                      SUPLA_ACTION_CAP_SHORT_PRESS_x1 | SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(ca_exec->counterSetCount(), 2);
  EXPECT_EQ(ca_exec->getOpenCounter(), 1);
  EXPECT_EQ(ca_exec->getRGBWCounter(), 1);
}

} /* namespace testing */
