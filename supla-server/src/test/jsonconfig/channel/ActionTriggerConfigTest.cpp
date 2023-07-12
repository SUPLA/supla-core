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
#include "jsonconfig/channel/action_trigger_config.h"
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

  config->set_active_cap(SUPLA_ACTION_CAP_HOLD);
  EXPECT_EQ(config->get_action_id(), 0);

  config->set_active_cap(SUPLA_ACTION_CAP_SHORT_PRESS_x2);
  EXPECT_EQ(config->get_action_id(), 0);

  config->set_capabilities(SUPLA_ACTION_CAP_HOLD |
                           SUPLA_ACTION_CAP_SHORT_PRESS_x2);

  config->set_active_cap(SUPLA_ACTION_CAP_HOLD);

  EXPECT_EQ(config->get_action_id(), ACTION_SHUT);
  EXPECT_EQ(config->get_subject_id(), 3611);
  EXPECT_EQ(config->get_source_device_id(), 0);
  EXPECT_EQ(config->get_source_channel_id(), 0);
  EXPECT_EQ(config->get_subject_type(), stChannel);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getPercentage) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":98}}}}}");

  EXPECT_EQ(config->get_percentage(), -1);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_percentage(), -1);

  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_percentage(), 98);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":110}}}}}");

  EXPECT_EQ(config->get_percentage(), -1);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":0}}}}}");

  EXPECT_EQ(config->get_percentage(), 0);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentag\":10}}}}}");

  EXPECT_EQ(config->get_percentage(), -1);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getBrightness) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightness\":15}}}}}");

  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_rgbw().Brightness, -1);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_rgbw().Brightness, 15);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightness\":110}}}}}");

  EXPECT_EQ(config->get_rgbw().Brightness, -1);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightness\":0}}}}}");

  EXPECT_EQ(config->get_rgbw().Brightness, 0);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"brightnes\":15}}}}}");

  EXPECT_EQ(config->get_rgbw().Brightness, -1);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getColorBrightness) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightness\":15}}}}"
      "}");

  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(config->get_rgbw().ColorBrightness, -1);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_rgbw().ColorBrightness, 15);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightness\":110}}}"
      "}}");

  EXPECT_EQ(config->get_rgbw().ColorBrightness, -1);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightness\":0}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw().ColorBrightness, 0);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"color_brightnes\":15}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw().ColorBrightness, -1);

  delete config;
}

TEST_F(ActionTriggerConfigTest, getColor) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"hue\":35}}}}"
      "}");

  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);
  EXPECT_EQ(config->get_rgbw().Color, (unsigned int)0);

  EXPECT_FALSE(config->get_rgbw().ColorRandom);

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_rgbw().Color, (unsigned int)0xFF9400);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"hue\":\"random\"}}}}"
      "}");

  unsigned int color = config->get_rgbw().Color;
  int a = 0;
  for (a = 0; a < 10; a++) {
    if (color != config->get_rgbw().Color) {
      break;
    }
  }
  EXPECT_NE(a, 10);

  EXPECT_TRUE(config->get_rgbw().ColorRandom);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":1551,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":80,\"param\":{\"hue\":\"white\"}}}}"
      "}");

  EXPECT_EQ(config->get_rgbw().Color, (unsigned int)0xFFFFFF);

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

  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x5);

  EXPECT_EQ(config->get_action_id(), ACTION_SET_RGBW_PARAMETERS);
  EXPECT_EQ(config->get_subject_id(), 1551);
  EXPECT_EQ(config->get_source_device_id(), 0);
  EXPECT_EQ(config->get_source_channel_id(), 0);
  EXPECT_EQ(config->get_subject_type(), stChannel);

  TAction_RGBW_Parameters rgbw = config->get_rgbw();
  EXPECT_EQ(rgbw.Brightness, (char)-1);
  EXPECT_EQ(rgbw.ColorBrightness, (char)44);
  EXPECT_EQ(rgbw.Color, (unsigned int)0xFF002A);
  EXPECT_FALSE(rgbw.ColorRandom);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionRevealPartially) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":3611,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":50,\"param\":{\"percentage\":65}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);
  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_action_id(), ACTION_REVEAL_PARTIALLY);
  EXPECT_EQ(config->get_subject_id(), 3611);
  EXPECT_EQ(config->get_source_device_id(), 0);
  EXPECT_EQ(config->get_source_channel_id(), 0);
  EXPECT_EQ(config->get_percentage(), 65);
  EXPECT_EQ(config->get_subject_type(), stChannel);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionShutPartially) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"TOGGLE_X1\":{\"subjectId\":45678,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":51,\"param\":{\"percentage\":20}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);
  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_action_id(), ACTION_SHUT_PARTIALLY);
  EXPECT_EQ(config->get_subject_id(), 45678);
  EXPECT_EQ(config->get_source_channel_id(), 0);
  EXPECT_EQ(config->get_percentage(), 20);
  EXPECT_EQ(config->get_subject_type(), stChannel);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionTurnOn) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":3329,\"subjectType\":"
      "\"channel\",\"action\":{\"id\":60,\"param\":[]}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x5);
  config->set_active_cap(SUPLA_ACTION_CAP_SHORT_PRESS_x5);

  EXPECT_EQ(config->get_action_id(), ACTION_TURN_ON);
  EXPECT_EQ(config->get_subject_id(), 3329);
  EXPECT_EQ(config->get_source_channel_id(), 0);
  EXPECT_EQ(config->get_subject_type(), stChannel);

  delete config;
}

TEST_F(ActionTriggerConfigTest, actionTurnOnTheGroup) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"actions\":{\"SHORT_PRESS_X5\":{\"subjectId\":3329,\"subjectType\":"
      "\"channelGroup\",\"action\":{\"id\":60,\"param\":[]}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_SHORT_PRESS_x5);
  config->set_active_cap(SUPLA_ACTION_CAP_SHORT_PRESS_x5);

  EXPECT_EQ(config->get_action_id(), ACTION_TURN_ON);
  EXPECT_EQ(config->get_subject_id(), 3329);
  EXPECT_EQ(config->get_source_channel_id(), 0);
  EXPECT_EQ(config->get_subject_type(), stChannelGroup);

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
  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_action_id(), ACTION_COPY);
  EXPECT_EQ(config->get_subject_id(), 55);
  EXPECT_EQ(config->get_source_channel_id(), 16868);
  EXPECT_EQ(config->get_subject_type(), stChannel);

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
  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_action_id(), ACTION_COPY);
  EXPECT_EQ(config->get_subject_id(), 31);
  EXPECT_EQ(config->get_source_device_id(), 0);
  EXPECT_EQ(config->get_source_channel_id(), 46868);
  EXPECT_EQ(config->get_subject_type(), stChannelGroup);

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
  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_action_id(), ACTION_COPY);
  EXPECT_EQ(config->get_subject_id(), 55);
  EXPECT_EQ(config->get_source_device_id(), 15);
  EXPECT_EQ(config->get_source_channel_id(), 16868);
  EXPECT_EQ(config->get_subject_type(), stChannel);

  delete config;
}

TEST_F(ActionTriggerConfigTest, setSubjectIdIfNotSet) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channel\",\"action\":{\"id\":"
      "10100,\"param\":{\"sourceDeviceId\":15,\"sourceChannelId\":16868}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TOGGLE_x1);
  config->set_active_cap(SUPLA_ACTION_CAP_TOGGLE_x1);

  EXPECT_EQ(config->get_action_id(), ACTION_COPY);
  EXPECT_EQ(config->get_subject_id(), 0);
  EXPECT_EQ(config->get_source_device_id(), 15);
  EXPECT_EQ(config->get_source_channel_id(), 16868);
  EXPECT_EQ(config->get_subject_type(), stChannel);

  config->set_channel_id_if_subject_not_set(123);
  EXPECT_EQ(config->get_subject_id(), 0);

  config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channel\",\"action\":{\"id\":"
      "10000,\"param\":{\"sourceDeviceId\":15,\"sourceChannelId\":16868}}}}}");

  EXPECT_EQ(config->get_subject_id(), 123);
  EXPECT_EQ(config->get_action_id(), ACTION_FORWARD_OUTSIDE);

  config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TOGGLE_X1\":{"
      "\"subjectType\":\"channelGroup\",\"subjectId\":31,\"action\":{\"id\":"
      "10000,\"param\":{\"sourceChannelId\":46868}}}}}");

  config->set_channel_id_if_subject_not_set(456);
  EXPECT_EQ(config->get_subject_id(), 31);
  EXPECT_EQ(config->get_action_id(), ACTION_FORWARD_OUTSIDE);

  delete config;
}

TEST_F(ActionTriggerConfigTest, sendPush) {
  action_trigger_config *config = new action_trigger_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config(
      "{\"disablesLocalOperation\":[],\"relatedChannelId\":null,"
      "\"hideInChannelsList\":false,\"actions\":{\"TURN_ON\":{\"subjectType\":"
      "\"notification\",\"subjectId\":272,\"action\":{\"id\":220,\"param\":{"
      "\"title\":\"Test 123\",\"body\":\"ABCD\",\"accessIds\":[5]}}}}}");

  config->set_capabilities(SUPLA_ACTION_CAP_TURN_ON);
  config->set_active_cap(SUPLA_ACTION_CAP_TURN_ON);

  EXPECT_EQ(config->get_action_id(), ACTION_SEND);
  EXPECT_EQ(config->get_subject_id(), 272);
  EXPECT_EQ(config->get_subject_type(), stPushNotification);

  delete config;
}

} /* namespace testing */
