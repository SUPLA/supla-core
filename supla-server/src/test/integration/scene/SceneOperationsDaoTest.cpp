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

#include <integration/scene/SceneOperationsDaoTest.h>

#include "log.h"  // NOLINT

namespace testing {

SceneOperationsDaoTest::SceneOperationsDaoTest() : IntegrationTest(), Test() {
  dao = NULL;
}

SceneOperationsDaoTest::~SceneOperationsDaoTest() {}

void SceneOperationsDaoTest::SetUp() {
  dao = new supla_scene_operations_dao();
  ASSERT_TRUE(dao != NULL);

  initTestDatabase();
  Test::SetUp();
}

void SceneOperationsDaoTest::TearDown() {
  if (dao) {
    delete dao;
    dao = NULL;
  }
  Test::TearDown();
}

TEST_F(SceneOperationsDaoTest, operationsForScene1) {
  supla_scene_operations *operations = dao->get_scene_operations(1);

  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 3);
    supla_scene_operation *operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_TURN_ON);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 140);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }

    operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 1000);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_OPEN);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 308);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }

    operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_SET_RGBW_PARAMETERS);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 170);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 10);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }
    delete operations;
  }
}

TEST_F(SceneOperationsDaoTest, operationsForScene2) {
  supla_scene_operations *operations = dao->get_scene_operations(2);
  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 2);
    supla_scene_operation *operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_TURN_ON);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 142);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }

    operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 5000);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_TURN_OFF);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 140);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }
    delete operations;
  }
}

TEST_F(SceneOperationsDaoTest, operationsForScene3) {
  supla_scene_operations *operations = dao->get_scene_operations(3);

  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 3);
    supla_scene_operation *operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_TURN_ON);
      EXPECT_EQ(cfg->get_subject_type(), stChannelGroup);
      EXPECT_EQ(cfg->get_subject_id(), 1);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }
    operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_SET_RGBW_PARAMETERS);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 171);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 6);
      EXPECT_EQ(cfg->get_rgbw().color, (unsigned int)0x0000FF);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 10);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }
    operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_EXECUTE);
      EXPECT_EQ(cfg->get_subject_type(), stScene);
      EXPECT_EQ(cfg->get_subject_id(), 1);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }
    delete operations;
  }
}

TEST_F(SceneOperationsDaoTest, operationsForScene4) {
  supla_scene_operations *operations = dao->get_scene_operations(4);

  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 1);
    supla_scene_operation *operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_COPY);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 140);
      EXPECT_EQ(cfg->get_source_device_id(), 73);
      EXPECT_EQ(cfg->get_source_channel_id(), 143);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }
    delete operations;
  }
}

TEST_F(SceneOperationsDaoTest, operationsForScene5) {
  supla_scene_operations *operations = dao->get_scene_operations(5);

  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 2);
    supla_scene_operation *operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_REVEAL_PARTIALLY);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 173);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 15);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }

    operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_SHUT_PARTIALLY);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 303);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 25);
      EXPECT_EQ(cfg->get_rgbw().brightness, 0);
      EXPECT_EQ(cfg->get_rgbw().color, 0U);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 0);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }
    delete operations;
  }
}

TEST_F(SceneOperationsDaoTest, operationsForScene6) {
  supla_scene_operations *operations = dao->get_scene_operations(6);

  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 2);
    supla_scene_operation *operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 0);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_SET_RGBW_PARAMETERS);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 171);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 50);
      EXPECT_EQ(cfg->get_rgbw().color, (unsigned int)0xFFFFFF);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 5);
      EXPECT_FALSE(cfg->get_rgbw().color_random);
      delete operation;
    }

    operation = operations->pop();
    EXPECT_TRUE(operation != NULL);
    if (operation) {
      EXPECT_EQ(operation->get_delay_ms(), 5000);
      supla_action_config *cfg = operation->get_action_config();
      EXPECT_EQ(cfg->get_action_id(), ACTION_SET_RGBW_PARAMETERS);
      EXPECT_EQ(cfg->get_subject_type(), stChannel);
      EXPECT_EQ(cfg->get_subject_id(), 171);
      EXPECT_EQ(cfg->get_source_device_id(), 0);
      EXPECT_EQ(cfg->get_source_channel_id(), 0);
      EXPECT_EQ(cfg->get_percentage(), 0);
      EXPECT_EQ(cfg->get_rgbw().brightness, 20);
      EXPECT_GT(cfg->get_rgbw().color, (unsigned int)0);
      EXPECT_EQ(cfg->get_rgbw().color_brightness, 15);
      EXPECT_TRUE(cfg->get_rgbw().color_random);
      delete operation;
    }
    delete operations;
  }
}

TEST_F(SceneOperationsDaoTest, getOperationsForDisabledScene) {
  supla_scene_operations *operations = dao->get_scene_operations(3);
  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 3);
    delete operations;
  }

  runSqlScript("DisableScenes3.sql");

  operations = dao->get_scene_operations(3);
  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 0);
    delete operations;
  }
}

TEST_F(SceneOperationsDaoTest, getOperationsForNonExistentScene) {
  supla_scene_operations *operations = dao->get_scene_operations(23);
  EXPECT_TRUE(operations != NULL);
  if (operations) {
    EXPECT_EQ(operations->count(), 0);
    delete operations;
  }
}

} /* namespace testing */
