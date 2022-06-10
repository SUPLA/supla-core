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
  scene.set_initiator_id(50);
  scene.set_initiator_name("Samsung Galaxy");
  scene.set_milliseconds_from_start(123456);
  scene.set_milliseconds_left(567890);

  EXPECT_EQ(scene.get_id(), 15);
  EXPECT_EQ(strncmp(scene.get_caption(), "Scene caption",
                    SUPLA_SCENE_CAPTION_MAXSIZE + 1),
            0);
  EXPECT_EQ(scene.get_user_icon_id(), 20);
  EXPECT_EQ(scene.get_alt_icon_id(), 30);
  EXPECT_EQ(scene.get_location_id(), 40);
  EXPECT_EQ(scene.get_initiator_id(), 50);
  EXPECT_EQ(strncmp(scene.get_initiator_name(), "Samsung Galaxy",
                    SUPLA_INITIATOR_NAME_MAXSIZE + 1),
            0);

  EXPECT_EQ(scene.get_milliseconds_from_start(), 123456U);
  EXPECT_EQ(scene.get_milliseconds_left(), 567890U);
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
  supla_client_scene scene(123);
  scene.set_initiator_id(456);
  scene.set_initiator_name("Samsung Galaxy");
  scene.set_milliseconds_from_start(2345);
  scene.set_milliseconds_left(67890);

  TSC_SuplaSceneState sc_scene_state = {};
  scene.convert(&sc_scene_state);

  EXPECT_EQ(sc_scene_state.EOL, 0);
  EXPECT_EQ(sc_scene_state.SceneId, 123);
  EXPECT_EQ(sc_scene_state.InitiatorId, 456);
  EXPECT_EQ(sc_scene_state.MillisecondsFromStart, 2345U);
  EXPECT_EQ(sc_scene_state.MillisecondsLeft, 67890U);

  EXPECT_EQ(
      sc_scene_state.InitiatorNameSize,
      strnlen(scene.get_initiator_name(), SUPLA_INITIATOR_NAME_MAXSIZE) + 1);

  EXPECT_EQ(strncmp(sc_scene_state.InitiatorName, scene.get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE + 1),
            0);
}

} /* namespace testing */
