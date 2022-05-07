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

#include "SceneRepositoryTest.h"

#include "log.h"  // NOLINT

namespace testing {

SceneRepositoryTest::SceneRepositoryTest() : IntegrationTest(), Test() {
  repo = NULL;
}

SceneRepositoryTest::~SceneRepositoryTest() {}

void SceneRepositoryTest::SetUp() {
  repo = new supla_scene_repository(2);
  ASSERT_TRUE(repo != NULL);

  initTestDatabase();
  Test::SetUp();
}

void SceneRepositoryTest::TearDown() {
  if (repo) {
    delete repo;
    repo = NULL;
  }
  Test::TearDown();
}

void SceneRepositoryTest::VerifyScene1(supla_scene *scene) {
  EXPECT_TRUE(scene != NULL);
  if (!scene) {
    return;
  }

  EXPECT_EQ(scene->get_id(), 1);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_caption(), "Scene #1");

  supla_scene_operations *operations =
      scene->get_operations() ? scene->get_operations()->clone() : NULL;
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

  delete scene;
}

void SceneRepositoryTest::VerifyScene2(supla_scene *scene) {
  EXPECT_TRUE(scene != NULL);
  if (!scene) {
    return;
  }

  EXPECT_EQ(scene->get_id(), 2);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_caption(), "Scene #2");

  supla_scene_operations *operations =
      scene->get_operations() ? scene->get_operations()->clone() : NULL;
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

  delete scene;
}

void SceneRepositoryTest::VerifyScene3(supla_scene *scene) {
  EXPECT_TRUE(scene != NULL);
  if (!scene) {
    return;
  }

  EXPECT_EQ(scene->get_id(), 3);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_caption(), "Scene #3");

  supla_scene_operations *operations =
      scene->get_operations() ? scene->get_operations()->clone() : NULL;
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

  delete scene;
}

void SceneRepositoryTest::VerifyScene4(supla_scene *scene) {
  EXPECT_TRUE(scene != NULL);
  if (!scene) {
    return;
  }

  EXPECT_EQ(scene->get_id(), 4);
  EXPECT_EQ(scene->get_location_id(), 3);
  EXPECT_EQ(scene->get_caption(), "Scene #4");

  supla_scene_operations *operations =
      scene->get_operations() ? scene->get_operations()->clone() : NULL;
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

  delete scene;
}

void SceneRepositoryTest::VerifyScene5(supla_scene *scene) {
  EXPECT_TRUE(scene != NULL);
  if (!scene) {
    return;
  }

  EXPECT_EQ(scene->get_id(), 5);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_caption(), "Scene #5");

  supla_scene_operations *operations =
      scene->get_operations() ? scene->get_operations()->clone() : NULL;
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

  delete scene;
}

void SceneRepositoryTest::VerifyScene6(supla_scene *scene) {
  EXPECT_TRUE(scene != NULL);
  if (!scene) {
    return;
  }

  EXPECT_EQ(scene->get_id(), 6);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_caption(), "Scene #6");

  supla_scene_operations *operations =
      scene->get_operations() ? scene->get_operations()->clone() : NULL;
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

  delete scene;
}

TEST_F(SceneRepositoryTest, getAllScenes) {
  std::vector<supla_scene *> scenes = repo->get_all_scenes();
  EXPECT_EQ(scenes.size(), 6U);

  supla_scene *scene = scenes.back();
  scenes.pop_back();

  VerifyScene6(scene);

  scene = scenes.back();
  scenes.pop_back();

  VerifyScene5(scene);

  scene = scenes.back();
  scenes.pop_back();

  VerifyScene4(scene);

  scene = scenes.back();
  scenes.pop_back();

  VerifyScene3(scene);

  scene = scenes.back();
  scenes.pop_back();

  VerifyScene2(scene);

  scene = scenes.back();
  scenes.pop_back();

  VerifyScene1(scene);
}

TEST_F(SceneRepositoryTest, disableAllScenesExceptOne) {
  runSqlScript("DisableAllScenesExceptOne.sql");

  std::vector<supla_scene *> scenes = repo->get_all_scenes();
  EXPECT_EQ(scenes.size(), 1U);

  supla_scene *scene = scenes.back();
  scenes.pop_back();

  VerifyScene3(scene);
}

TEST_F(SceneRepositoryTest, getSingleScene) {
  supla_scene *scene = repo->get_scene(5);
  VerifyScene5(scene);
}

TEST_F(SceneRepositoryTest, getNonExistentScene) {
  supla_scene *scene = repo->get_scene(550);
  EXPECT_TRUE(scene == NULL);
}

TEST_F(SceneRepositoryTest, getAllScenesForNonExistentUser) {
  delete repo;
  repo = new supla_scene_repository(23456);
  ASSERT_TRUE(repo != NULL);

  std::vector<supla_scene *> scenes = repo->get_all_scenes();
  EXPECT_EQ(scenes.size(), 0U);
}

} /* namespace testing */
