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

#include "ActionTriggerConfigTest.h"

#include "TestHelper.h"
#include "channeljsonconfig/action_trigger_config.h"
#include "proto.h"

namespace testing {

ActionTriggerConfigTest::ActionTriggerConfigTest(void) {}
ActionTriggerConfigTest::~ActionTriggerConfigTest(void) {}

TEST_F(ActionTriggerConfigTest, root) {
  channel_json_config *c1 = new channel_json_config();
  action_trigger_config *c2 = new action_trigger_config(c1);

  EXPECT_TRUE(c1 != NULL);

  if (!c1 || !c2) {
    if (c1) {
      delete c1;
    }

    if (c2) {
      delete c2;
    }
  }

  char *str1 = c1->get_properties();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    EXPECT_EQ(strncmp(str1, "{}", 5), 0);
    free(str1);
    str1 = NULL;
  }

  str1 = c2->get_properties();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    EXPECT_EQ(strncmp(str1, "{}", 5), 0);
    free(str1);
    str1 = NULL;
  }

  c2->set_capabilities(SUPLA_ACTION_CAP_HOLD);

  str1 = c1->get_properties();
  EXPECT_TRUE(str1 != NULL);

  char *str2 = c2->get_properties();
  EXPECT_TRUE(str2 != NULL);

  if (str1 && str2) {
    EXPECT_NE(strncmp(str1, "{}", 5), 0);
    EXPECT_EQ(strncmp(str1, str2, 1000), 0);
  }

  if (str1) {
    free(str1);
    str1 = NULL;
  }

  if (str2) {
    free(str2);
    str2 = NULL;
  }

  str1 = c2->get_user_config();
  EXPECT_TRUE(str1 != NULL);

  if (str1) {
    EXPECT_EQ(strncmp(str1, "{}", 5), 0);
    free(str1);
    str1 = NULL;
  }

  delete c1;
  delete c2;
}

TEST_F(ActionTriggerConfigTest, allCaps) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_TRUE(config->set_capabilities(0xFFFFFFFF));
  EXPECT_FALSE(config->set_capabilities(0xFFFFFFFF));

  unsigned int all =
      SUPLA_ACTION_CAP_TURN_ON | SUPLA_ACTION_CAP_TURN_OFF |
      SUPLA_ACTION_CAP_TOGGLE_x1 | SUPLA_ACTION_CAP_TOGGLE_x2 |
      SUPLA_ACTION_CAP_TOGGLE_x3 | SUPLA_ACTION_CAP_TOGGLE_x4 |
      SUPLA_ACTION_CAP_TOGGLE_x5 | SUPLA_ACTION_CAP_HOLD |
      SUPLA_ACTION_CAP_SHORT_PRESS_x1 | SUPLA_ACTION_CAP_SHORT_PRESS_x2 |
      SUPLA_ACTION_CAP_SHORT_PRESS_x3 | SUPLA_ACTION_CAP_SHORT_PRESS_x4 |
      SUPLA_ACTION_CAP_SHORT_PRESS_x5;

  EXPECT_EQ(config->get_capabilities(), all);

  char *str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    // TestHelper::printEscaped(str);
    EXPECT_EQ(
        strncmp(
            str,
            "{\"actionTriggerCapabilities\":[\"TURN_ON\",\"TURN_OFF\",\"TOGGLE_"
            "X1\",\"TOGGLE_X2\",\"TOGGLE_X3\",\"TOGGLE_X4\",\"TOGGLE_X5\","
            "\"HOLD\",\"SHORT_PRESS_X1\",\"SHORT_PRESS_X2\",\"SHORT_PRESS_"
            "X3\",\"SHORT_PRESS_X4\",\"SHORT_PRESS_X5\"]}",
            1000),
        0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ActionTriggerConfigTest, allCapabilitiesThatCanDisableLocalOperation) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  EXPECT_TRUE(config->set_caps_that_disables_local_operation(0xFFFFFFFF));
  EXPECT_FALSE(config->set_caps_that_disables_local_operation(0xFFFFFFFF));

  unsigned int all =
      SUPLA_ACTION_CAP_TURN_ON | SUPLA_ACTION_CAP_TURN_OFF |
      SUPLA_ACTION_CAP_TOGGLE_x1 | SUPLA_ACTION_CAP_TOGGLE_x2 |
      SUPLA_ACTION_CAP_TOGGLE_x3 | SUPLA_ACTION_CAP_TOGGLE_x4 |
      SUPLA_ACTION_CAP_TOGGLE_x5 | SUPLA_ACTION_CAP_HOLD |
      SUPLA_ACTION_CAP_SHORT_PRESS_x1 | SUPLA_ACTION_CAP_SHORT_PRESS_x2 |
      SUPLA_ACTION_CAP_SHORT_PRESS_x3 | SUPLA_ACTION_CAP_SHORT_PRESS_x4 |
      SUPLA_ACTION_CAP_SHORT_PRESS_x5;

  EXPECT_EQ(config->get_caps_that_disables_local_operation(), all);

  char *str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    // TestHelper::printEscaped(str);
    EXPECT_EQ(
        strncmp(
            str,
            "{\"disablesLocalOperation\":[\"TURN_ON\",\"TURN_OFF\",\"TOGGLE_"
            "X1\",\"TOGGLE_X2\",\"TOGGLE_X3\",\"TOGGLE_X4\",\"TOGGLE_X5\","
            "\"HOLD\",\"SHORT_PRESS_X1\",\"SHORT_PRESS_X2\",\"SHORT_PRESS_"
            "X3\",\"SHORT_PRESS_X4\",\"SHORT_PRESS_X5\"]}",
            1000),
        0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ActionTriggerConfigTest, amodificationAmongOtherParameters) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_properties("{\"a\":true}");

  EXPECT_TRUE(config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2));
  EXPECT_FALSE(config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x2));

  char *str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    // TestHelper::printEscaped(str);

    EXPECT_EQ(
        strncmp(str,
                "{\"a\":true,\"actionTriggerCapabilities\":[\"TOGGLE_X2\"]}",
                1000),
        0);

    free(str);
    str = NULL;
  }

  config->set_properties(
      "{\"actionTriggerCapabilities\":[\"TOGGLE_X3\"],\"a\":true}");
  EXPECT_EQ(config->get_capabilities(),
            (unsigned int)SUPLA_ACTION_CAP_TOGGLE_x3);

  EXPECT_FALSE(config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x3));

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    // TestHelper::printEscaped(str);

    EXPECT_EQ(
        strncmp(str,
                "{\"actionTriggerCapabilities\":[\"TOGGLE_X3\"],\"a\":true}",
                1000),
        0);

    free(str);
    str = NULL;
  }

  EXPECT_TRUE(config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x3 |
                                       SUPLA_ACTION_CAP_TOGGLE_x1 |
                                       SUPLA_ACTION_CAP_TOGGLE_x4));

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    // TestHelper::printEscaped(str);

    EXPECT_EQ(
        strncmp(str,
                "{\"actionTriggerCapabilities\":[\"TOGGLE_X1\",\"TOGGLE_X3\","
                "\"TOGGLE_X4\"],\"a\":true}",
                1000),
        0);

    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ActionTriggerConfigTest, readActiveTriggers) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":1,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":10,\"param\":[]}},\"HOLD\":{"
      "\"subjectId\":1,\"subjectType\":\"scene\",\"action\":{\"id\":3000,"
      "\"param\":[]}}}}");

  EXPECT_EQ(config->get_active_actions(), (unsigned int)0);

  config->set_properties("{\"actionTriggerCapabilities\":[\"HOLD\"]}");

  EXPECT_EQ(config->get_active_actions(), (unsigned int)SUPLA_ACTION_CAP_HOLD);

  config->set_properties(
      "{\"actionTriggerCapabilities\":[\"HOLD\", \"SHORT_PRESS_X1\", "
      "\"SHORT_PRESS_X5\"]}");

  EXPECT_EQ(
      config->get_active_actions(),
      (unsigned int)(SUPLA_ACTION_CAP_SHORT_PRESS_x5 | SUPLA_ACTION_CAP_HOLD));

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":1,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":10,\"param\":[]}}}}");

  EXPECT_EQ(config->get_active_actions(),
            (unsigned int)(SUPLA_ACTION_CAP_SHORT_PRESS_x5));

  config->set_user_config("{}");

  EXPECT_EQ(config->get_active_actions(), (unsigned int)0);

  config->set_properties(
      "{\"actionTriggerCapabilities\":[\"HOLD\", \"SHORT_PRESS_X1\", "
      "\"TOGGLE_X1\"]}");

  config->set_user_config(
      "{\"relatedChannelId\":4266,\"hideInChannelsList\":true,\"actions\":{"
      "\"TOGGLE_X1\":{\"subjectType\":\"other\",\"action\":{\"id\":10200}}}}");

  EXPECT_EQ(config->get_active_actions(),
            (unsigned int)(SUPLA_ACTION_CAP_TOGGLE_x1));

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionShutRevelal) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"HOLD\":{\"subjectId\":3611,\"subjectType\":\"channel\","
      "\"action\":{\"id\":30,\"param\":[]}},\"SHORT_PRESS_X2\":{\"subjectId\":"
      "3611,\"subjectType\":\"channel\",\"action\":{\"id\":40,\"param\":[]}}}"
      "}");

  _at_config_action_t action =
      config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_HOLD);
  EXPECT_EQ(action.actionId, 0);

  action = config->get_action_assigned_to_capability(
      SUPLA_ACTION_CAP_SHORT_PRESS_x2);
  EXPECT_EQ(action.actionId, 0);

  config->set_capabilities(SUPLA_ACTION_CAP_HOLD |
                           SUPLA_ACTION_CAP_SHORT_PRESS_x2);

  action = config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_HOLD);
  EXPECT_EQ(action.actionId, ACTION_SHUT);
  EXPECT_EQ(action.subjectId, 3611);
  EXPECT_EQ(action.sourceChannelId, 0);
  EXPECT_FALSE(action.channelGroup);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getPercentage) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":98}}}}}");

  EXPECT_EQ(config->get_percentage(SUPLA_ACTION_CAP_TOGGLE_x1), -1);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_percentage(SUPLA_ACTION_CAP_TOGGLE_x1), 98);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":110}}}}}");

  EXPECT_EQ(config->get_percentage(SUPLA_ACTION_CAP_TOGGLE_x1), -1);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":0}}}}}");

  EXPECT_EQ(config->get_percentage(SUPLA_ACTION_CAP_TOGGLE_x1), 0);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentag\":10}}}}}");

  EXPECT_EQ(config->get_percentage(SUPLA_ACTION_CAP_TOGGLE_x1), -1);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getBrightness) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightness\":15}}}}}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).brightness, -1);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).brightness, 15);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightness\":110}}}}}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).brightness, -1);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightness\":0}}}}}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).brightness, 0);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightnes\":15}}}}}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).brightness, -1);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getColorBrightness) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightness\":15}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color_brightness, -1);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color_brightness, 15);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightness\":110}}}"
      "}}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color_brightness, -1);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightness\":0}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color_brightness, 0);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightnes\":15}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color_brightness, -1);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getColor) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"hue\":35}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color,
            (unsigned int)0);

  EXPECT_FALSE(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color_random);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color,
            (unsigned int)0xFF9400);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"hue\":\"random\"}}}}"
      "}");

  unsigned int color = config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color;
  int a = 0;
  for (a = 0; a < 10; a++) {
    if (color != config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color) {
      break;
    }
  }
  EXPECT_NE(a, 10);

  EXPECT_TRUE(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color_random);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"hue\":\"white\"}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x1).color,
            (unsigned int)0xFFFFFF);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionSetRGBW) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X5\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"hue\":350,\"color_"
      "brightness\":44}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x5);

  _at_config_action_t action =
      config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_TOGGLE_x5);
  EXPECT_EQ(action.actionId, ACTION_SET_RGBW_PARAMETERS);
  EXPECT_EQ(action.subjectId, 1551);
  EXPECT_EQ(action.sourceChannelId, 0);
  EXPECT_FALSE(action.channelGroup);

  _at_config_rgbw_t rgbw = config->get_rgbw(SUPLA_ACTION_CAP_TOGGLE_x5);
  EXPECT_EQ(rgbw.brightness, (char)-1);
  EXPECT_EQ(rgbw.color_brightness, (char)44);
  EXPECT_EQ(rgbw.color, (unsigned int)0xFF002A);
  EXPECT_FALSE(rgbw.color_random);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionRevealPartially) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":65}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  _at_config_action_t action =
      config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(action.actionId, ACTION_REVEAL_PARTIALLY);
  EXPECT_EQ(action.subjectId, 3611);
  EXPECT_EQ(action.sourceChannelId, 0);
  EXPECT_EQ(config->get_percentage(SUPLA_ACTION_CAP_TOGGLE_x1), 65);
  EXPECT_FALSE(action.channelGroup);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionShutPartially) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":45678,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":51,\"param\":{\"percentage\":20}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  _at_config_action_t action =
      config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(action.actionId, ACTION_SHUT_PARTIALLY);
  EXPECT_EQ(action.subjectId, 45678);
  EXPECT_EQ(action.sourceChannelId, 0);
  EXPECT_EQ(config->get_percentage(SUPLA_ACTION_CAP_TOGGLE_x1), 20);
  EXPECT_FALSE(action.channelGroup);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionTurnOn) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":3329,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":60,\"param\":[]}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x5);

  _at_config_action_t action = config->get_action_assigned_to_capability(
      SUPLA_ACTION_CAP_SHORT_PRESS_x5);
  EXPECT_EQ(action.actionId, ACTION_TURN_ON);
  EXPECT_EQ(action.subjectId, 3329);
  EXPECT_EQ(action.sourceChannelId, 0);
  EXPECT_FALSE(action.channelGroup);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionTurnOnTheGroup) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":3329,\"subjectType\":"
      "\"channelGroup\",\"action\":{\"id\":60,\"param\":[]}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x5);

  _at_config_action_t action = config->get_action_assigned_to_capability(
      SUPLA_ACTION_CAP_SHORT_PRESS_x5);
  EXPECT_EQ(action.actionId, ACTION_TURN_ON);
  EXPECT_EQ(action.subjectId, 3329);
  EXPECT_EQ(action.sourceChannelId, 0);
  EXPECT_TRUE(action.channelGroup);

  delete config;
}

TEST_F(ActionTriggerConfigTest, checkingIfTheChannelIdExists) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":3329,\"subjectType\":"
      "\"channelGroup\",\"action\":{\"id\":60,\"param\":[]}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x5);

  EXPECT_FALSE(config->channel_exists(3329));

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":3329,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":60,\"param\":[]}}}}");

  EXPECT_TRUE(config->channel_exists(3329));

  config->set_capabilities(0);

  EXPECT_FALSE(config->channel_exists(3329));

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":3329,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":60,\"param\":[]}},\"SHORT_PRESS_X4\":{"
      "\"subjectId\":3,\"subjectType\":\"channel\",\"action\":{\"id\":60,"
      "\"param\":[]}}}}");

  EXPECT_FALSE(config->channel_exists(3));

  config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x5);

  EXPECT_FALSE(config->channel_exists(3));

  config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x5 |
                           SUPLA_ACTION_CAP_SHORT_PRESS_x4);

  EXPECT_TRUE(config->channel_exists(3));

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionCopy) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channel\",\"subjectId\":55,\"action\":{\"id\":"
      "10100,\"param\":{\"sourceChannelId\":16868}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  _at_config_action_t action =
      config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(action.actionId, ACTION_COPY);
  EXPECT_EQ(action.subjectId, 55);
  EXPECT_EQ(action.sourceChannelId, 16868);
  EXPECT_FALSE(action.channelGroup);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionCopyToTheChannelGroup) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channelGroup\",\"subjectId\":31,\"action\":{\"id\":"
      "10100,\"param\":{\"sourceChannelId\":46868}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  _at_config_action_t action =
      config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(action.actionId, ACTION_COPY);
  EXPECT_EQ(action.subjectId, 31);
  EXPECT_EQ(action.sourceDeviceId, 0);
  EXPECT_EQ(action.sourceChannelId, 46868);
  EXPECT_TRUE(action.channelGroup);

  delete config;
}

TEST_F(ActionTriggerConfigTest, sourceIds) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channel\",\"subjectId\":55,\"action\":{\"id\":"
      "10100,\"param\":{\"sourceDeviceId\":15,\"sourceChannelId\":16868}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  _at_config_action_t action =
      config->get_action_assigned_to_capability(SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(action.actionId, ACTION_COPY);
  EXPECT_EQ(action.subjectId, 55);
  EXPECT_EQ(action.sourceDeviceId, 15);
  EXPECT_EQ(action.sourceChannelId, 16868);
  EXPECT_FALSE(action.channelGroup);

  delete config;
}

} /* namespace testing */
