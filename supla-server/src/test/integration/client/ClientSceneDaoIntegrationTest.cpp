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

#include <integration/client/ClientSceneDaoIntegrationTest.h>

#include "log.h"  // NOLINT
#include "proto.h"

namespace testing {

using std::list;

ClientSceneDaoIntegrationTest::ClientSceneDaoIntegrationTest()
    : IntegrationTest(), Test() {
  dao = nullptr;
  dba = nullptr;
}

ClientSceneDaoIntegrationTest::~ClientSceneDaoIntegrationTest() {}

void ClientSceneDaoIntegrationTest::SetUp() {
  dba = new supla_db_access_provider();
  dao = new supla_client_scene_dao(dba);
  ASSERT_TRUE(dao != nullptr);

  initTestDatabase();
  Test::SetUp();
}

void ClientSceneDaoIntegrationTest::TearDown() {
  if (dao) {
    delete dao;
    dao = nullptr;
  }

  if (dba) {
    delete dba;
    dba = nullptr;
  }

  Test::TearDown();
}

TEST_F(ClientSceneDaoIntegrationTest, getScenesForNonExistentClient) {
  list<supla_client_scene *> scenes = dao->get_all_scenes(2, 2);
  EXPECT_EQ(scenes.size(), 0U);
}

TEST_F(ClientSceneDaoIntegrationTest, getAllScenes) {
  runSqlScript("SetUserIconForSceneWithId2.sql");
  runSqlScript("ChangeLocationForSceneWithId2.sql");
  list<supla_client_scene *> scenes = dao->get_all_scenes(2, 1);

  EXPECT_EQ(scenes.size(), 6U);

  supla_client_scene *scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 1);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #1", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
  scenes.pop_front();

  scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 2);
  EXPECT_EQ(scene->get_user_icon_id(), 26);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(scene->get_location_id(), 3);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #2", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
  scenes.pop_front();

  scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 3);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #3", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
  scenes.pop_front();

  scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 4);
  EXPECT_EQ(scene->get_location_id(), 3);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #4", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
  scenes.pop_front();

  scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 5);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #5", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
  scenes.pop_front();

  scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 6);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #6", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
}

TEST_F(ClientSceneDaoIntegrationTest, disableAllScenesExceptOne) {
  runSqlScript("DisableAllScenesExceptOne.sql");

  list<supla_client_scene *> scenes = dao->get_all_scenes(2, 1);

  EXPECT_EQ(scenes.size(), 1U);

  supla_client_scene *scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 3);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #3", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
}

TEST_F(ClientSceneDaoIntegrationTest, hideAllScenesExceptOne) {
  runSqlScript("HideAllScenesExceptOne.sql");

  list<supla_client_scene *> scenes = dao->get_all_scenes(2, 1);

  EXPECT_EQ(scenes.size(), 1U);

  supla_client_scene *scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 2);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(scene->get_alt_icon_id(), 0);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #2", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
}

TEST_F(ClientSceneDaoIntegrationTest, setAltIcon) {
  runSqlScript("DisableAllScenesExceptOne.sql");
  runSqlScript("SetAltIconForEnabledScenes.sql");

  list<supla_client_scene *> scenes = dao->get_all_scenes(2, 1);

  EXPECT_EQ(scenes.size(), 1U);

  supla_client_scene *scene = scenes.front();
  EXPECT_EQ(scene->get_id(), 3);
  EXPECT_EQ(scene->get_location_id(), 2);
  EXPECT_EQ(scene->get_user_icon_id(), 0);
  EXPECT_EQ(scene->get_alt_icon_id(), 4);
  EXPECT_EQ(
      strncmp(scene->get_caption(), "Scene #3", SUPLA_SCENE_CAPTION_MAXSIZE),
      0);

  delete scene;
}

} /* namespace testing */
