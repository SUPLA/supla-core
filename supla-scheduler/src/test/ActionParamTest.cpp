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
#include <list>

#include "ActionParamTest.h"
#include "action_copy.h"
#include "action_rgb.h"
#include "action_set.h"
#include "action_shut.h"

namespace testing {

ActionParamTest::ActionParamTest() { worker = NULL; }

ActionParamTest::~ActionParamTest() {}

void ActionParamTest::SetUp() {
  Test::SetUp();

  worker = new WorkerMock(NULL);

  ON_CALL(*worker, get_channel_func).WillByDefault(Return(0));
  ON_CALL(*worker, get_id).WillByDefault(Return(0));
  ON_CALL(*worker, it_applies_to_scene).WillByDefault(Return(false));
  ON_CALL(*worker, get_retry_count).WillByDefault(Return(0));
  ON_CALL(*worker, channel_group).WillByDefault(Return(false));
  ON_CALL(*worker, get_action_param).WillByDefault(Return((const char *)NULL));
  ON_CALL(*worker, retry_when_fail).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_get_opening_sensor_value).WillByDefault(Return(0));
  ON_CALL(*worker, ipcc_set_char_value).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_get_char_value).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_get_rgbw_value).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_set_rgbw_value).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_get_valve_value).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_set_digiglass_value).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_get_digiglass_value).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_action_copy).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_execute_scene).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_interrupt_scene).WillByDefault(Return(false));
  ON_CALL(*worker, ipcc_is_connected).WillByDefault(Return(false));
}

void ActionParamTest::TearDown() {
  Test::TearDown();
  delete worker;
}

TEST_F(ActionParamTest, time) {
  for (std::list<AbstractActionFactory *>::iterator it =
           AbstractActionFactory::factories.begin();
       it != AbstractActionFactory::factories.end(); it++) {
    s_worker_action *action = (*it)->create(worker);
    ASSERT_FALSE(action == NULL);

    EXPECT_GE(action->waiting_time_to_retry(), MIN_RETRY_TIME);
    EXPECT_GE(action->waiting_time_to_check(), MIN_CHECK_TIME);

    int diff =
        action->waiting_time_to_retry() - action->waiting_time_to_check();
    EXPECT_GT(diff, 0);

    EXPECT_LT(action->get_max_time(), 280);  // Max time 4 min 40 sec.

    delete action;
  }
}

TEST_F(ActionParamTest, parsePercentage) {
  s_worker_action_shut *action = new s_worker_action_shut(worker);
  EXPECT_FALSE(action == NULL);

  if (action) {
    char p = 0;
    std::string action_param;

    action_param = "{\"percentage\":45}";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_TRUE(action->parse_percentage(&p));
    EXPECT_EQ(p, 45);

    p = 0;
    action_param = "{\"perCentaGe\":80}";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_TRUE(action->parse_percentage(&p));
    EXPECT_EQ(p, 80);

    action_param = "{\"percentage\":110}";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_FALSE(action->parse_percentage(&p));

    action_param = "{\"percentage\":-1}";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_FALSE(action->parse_percentage(&p));

    action_param = "";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_FALSE(action->parse_percentage(&p));

    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return((const char *)NULL));

    EXPECT_FALSE(action->parse_percentage(&p));

    delete action;
  }
}

TEST_F(ActionParamTest, parseRgb) {
  s_worker_action_rgb *action = new s_worker_action_rgb(worker);
  EXPECT_FALSE(action == NULL);

  std::string action_param;

  action_param = "{\"brightness\":33,\"hue\":0,\"color_brightness\":28}";
  EXPECT_CALL(*worker, get_action_param)
      .WillRepeatedly(Return(action_param.c_str()));

  int color = -1;
  char color_brightness = -1;
  char brightness = -1;
  bool random = false;

  EXPECT_TRUE(action->parse_rgbw_params(&color, &color_brightness, &brightness,
                                        &random));

  EXPECT_EQ(color, 0xFF0000);
  EXPECT_EQ(color_brightness, 28);
  EXPECT_EQ(brightness, 33);
  EXPECT_FALSE(random);

  action_param = "{\"brightness\":15,\"hue\":244,\"color_brightness\":48}";
  EXPECT_CALL(*worker, get_action_param)
      .WillRepeatedly(Return(action_param.c_str()));

  color = -1;
  color_brightness = -1;
  brightness = -1;
  random = false;

  EXPECT_TRUE(action->parse_rgbw_params(&color, &color_brightness, &brightness,
                                        &random));

  EXPECT_EQ(color, 0x1000FF);
  EXPECT_EQ(color_brightness, 48);
  EXPECT_EQ(brightness, 15);
  EXPECT_FALSE(random);

  action_param =
      "{\"brightness\":31,\"hue\":\"random\",\"color_brightness\":21}";
  EXPECT_CALL(*worker, get_action_param)
      .WillRepeatedly(Return(action_param.c_str()));

  color = -1;
  color_brightness = -1;
  brightness = -1;
  random = false;

  EXPECT_TRUE(action->parse_rgbw_params(&color, &color_brightness, &brightness,
                                        &random));

  EXPECT_EQ(color_brightness, 21);
  EXPECT_EQ(brightness, 31);
  EXPECT_TRUE(random);

  action_param =
      "{\"brightness\":32,\"hue\":\"white\",\"color_brightness\":22}";
  EXPECT_CALL(*worker, get_action_param)
      .WillRepeatedly(Return(action_param.c_str()));

  color = -1;
  color_brightness = -1;
  brightness = -1;
  random = false;

  EXPECT_TRUE(action->parse_rgbw_params(&color, &color_brightness, &brightness,
                                        &random));

  EXPECT_EQ(color, 0xFFFFFF);
  EXPECT_EQ(color_brightness, 22);
  EXPECT_EQ(brightness, 32);
  EXPECT_FALSE(random);

  action_param = "{\"brightness\":32}";
  EXPECT_CALL(*worker, get_action_param)
      .WillRepeatedly(Return(action_param.c_str()));

  color = -1;
  color_brightness = -1;
  brightness = -1;
  random = false;

  EXPECT_TRUE(action->parse_rgbw_params(&color, &color_brightness, &brightness,
                                        &random));

  EXPECT_EQ(color, 0);
  EXPECT_EQ(color_brightness, 0);
  EXPECT_EQ(brightness, 32);
  EXPECT_FALSE(random);

  action_param = "{\"color_brightness\":32}";
  EXPECT_CALL(*worker, get_action_param)
      .WillRepeatedly(Return(action_param.c_str()));

  color = -1;
  color_brightness = -1;
  brightness = -1;
  random = false;

  EXPECT_TRUE(action->parse_rgbw_params(&color, &color_brightness, &brightness,
                                        &random));

  EXPECT_EQ(color, 0);
  EXPECT_EQ(color_brightness, 32);
  EXPECT_EQ(brightness, 0);
  EXPECT_FALSE(random);

  action_param = "{\"hue\":\"white\"}";
  EXPECT_CALL(*worker, get_action_param)
      .WillRepeatedly(Return(action_param.c_str()));

  color = -1;
  color_brightness = -1;
  brightness = -1;
  random = false;

  EXPECT_TRUE(action->parse_rgbw_params(&color, &color_brightness, &brightness,
                                        &random));

  EXPECT_EQ(color, 0xFFFFFF);
  EXPECT_EQ(color_brightness, 0);
  EXPECT_EQ(brightness, 0);
  EXPECT_FALSE(random);

  if (action) {
    delete action;
  }
}

TEST_F(ActionParamTest, copyActionParams) {
  s_worker_action_copy *action = new s_worker_action_copy(worker);
  EXPECT_FALSE(action == NULL);

  if (action) {
    int source_device_id = 0;
    int source_channel_id = 0;
    std::string action_param;

    action_param = "{\"sourceChannelId\":46865}";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_TRUE(action->get_params(&source_device_id, &source_channel_id));
    EXPECT_EQ(source_device_id, 0);
    EXPECT_EQ(source_channel_id, 46865);

    action_param = "{\"sourceDeviceId\":67, \"sourceChannelId\":46865}";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_TRUE(action->get_params(&source_device_id, &source_channel_id));
    EXPECT_EQ(source_device_id, 67);
    EXPECT_EQ(source_channel_id, 46865);

    action_param = "{\"sourceDeviceId\":67, \"sourceChannelId\":0}";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_FALSE(action->get_params(&source_device_id, &source_channel_id));

    action_param = "";
    EXPECT_CALL(*worker, get_action_param)
        .WillRepeatedly(Return(action_param.c_str()));

    EXPECT_FALSE(action->get_params(&source_device_id, &source_channel_id));

    delete action;
  }
}

}  // namespace testing
