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

#include "client/ClientSceneTest.h"

#include "client/client_scene.h"

namespace testing {

ClientSceneTest::ClientSceneTest() : Test() {}

ClientSceneTest::~ClientSceneTest() {}

TEST_F(ClientSceneTest, settersAndGetters) {
  supla_client_scene scene(15);
  scene.set_caption("Scene caption");
  scene.set_user_icon_id(20);
  scene.set_alt_icon_id(30);
  scene.set_location_id(40);

  char initiatorName[] = "Samsung Galaxy";

  struct timeval now = {};
  gettimeofday(&now, NULL);
  now.tv_sec -= 5;
  supla_scene_state state(supla_caller(ctClient, 50, initiatorName), now,
                          10000);
  scene.set_state(state);

  EXPECT_EQ(scene.get_id(), 15);
  EXPECT_EQ(strncmp(scene.get_caption(), "Scene caption",
                    SUPLA_SCENE_CAPTION_MAXSIZE + 1),
            0);
  EXPECT_EQ(scene.get_user_icon_id(), 20);
  EXPECT_EQ(scene.get_alt_icon_id(), 30);
  EXPECT_EQ(scene.get_location_id(), 40);
  EXPECT_EQ(scene.get_state().get_initiator_id(), 50);
  EXPECT_EQ(strncmp(scene.get_state().get_initiator_name(), initiatorName,
                    SUPLA_INITIATOR_NAME_MAXSIZE + 1),
            0);

  EXPECT_GE(scene.get_state().get_milliseconds_from_start(), 5000U);
  EXPECT_LE(scene.get_state().get_milliseconds_from_start(), 5200U);
  EXPECT_LE(scene.get_state().get_milliseconds_left(), 5000U);
  EXPECT_GE(scene.get_state().get_milliseconds_left(), 4800U);
}

TEST_F(ClientSceneTest, convertScene) {
  supla_client_scene scene(123);
  scene.set_caption("Scene caption #123");
  scene.set_user_icon_id(2);
  scene.set_alt_icon_id(3);
  scene.set_location_id(4);

  TSC_SuplaScene sc_scene = {};
  scene.convert(&sc_scene);

  EXPECT_EQ(sc_scene.EOL, 0);
  EXPECT_EQ(sc_scene.Id, 123);
  EXPECT_EQ(sc_scene.LocationId, 4);
  EXPECT_EQ(sc_scene.AltIcon, 3);
  EXPECT_EQ(sc_scene.UserIcon, 2);
  EXPECT_EQ(sc_scene.CaptionSize,
            strnlen(scene.get_caption(), SUPLA_SCENE_CAPTION_MAXSIZE) + 1);
  EXPECT_EQ(strncmp(sc_scene.Caption, scene.get_caption(),
                    SUPLA_SCENE_CAPTION_MAXSIZE + 1),
            0);
}

TEST_F(ClientSceneTest, convertSceneState) {
  char initiatorName[] = "Samsung Galaxy";

  struct timeval now = {};
  gettimeofday(&now, NULL);
  now.tv_sec -= 15;
  supla_scene_state state(supla_caller(ctClient, 456, initiatorName), now,
                          60000);

  supla_client_scene scene(123);
  scene.set_state(state);

  TSC_SuplaSceneState sc_scene_state = {};
  scene.convert(&sc_scene_state);

  EXPECT_EQ(sc_scene_state.EOL, 0);
  EXPECT_EQ(sc_scene_state.SceneId, 123);
  EXPECT_EQ(sc_scene_state.InitiatorId, 456);
  EXPECT_GE(sc_scene_state.MillisecondsFromStart, 15000U);
  EXPECT_LE(sc_scene_state.MillisecondsFromStart, 15200U);
  EXPECT_LE(sc_scene_state.MillisecondsLeft, 45000U);
  EXPECT_GE(sc_scene_state.MillisecondsLeft, 44800U);

  EXPECT_EQ(sc_scene_state.InitiatorNameSize,
            strnlen(scene.get_state().get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE) +
                1);

  EXPECT_EQ(strncmp(sc_scene_state.InitiatorName,
                    scene.get_state().get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE + 1),
            0);
}

} /* namespace testing */
