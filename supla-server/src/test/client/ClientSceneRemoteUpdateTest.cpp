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

#include "client/ClientSceneRemoteUpdateTest.h"

#include "client/client_scene.h"
#include "client/client_scene_change_indicator.h"
#include "client/client_scene_remote_updater.h"
#include "doubles/SrpcAdapterMock.h"
#include "doubles/distributedobjects/DObjectsMock.h"
#include "scene/scene_state.h"

namespace testing {

ClientSceneRemoteUpdateTest::ClientSceneRemoteUpdateTest() : Test() {}

ClientSceneRemoteUpdateTest::~ClientSceneRemoteUpdateTest() {}

TEST_F(ClientSceneRemoteUpdateTest, countOfScenesOverPackItemLimit) {
  SrpcAdapterMock srpcAdapter(NULL);

  TSC_SuplaScenePack scenePack = {};

  supla_client_scene_remote_updater remoteUpdater(&srpcAdapter);
  DObjectsMock scenes(&remoteUpdater);

  short count = SUPLA_SCENE_PACK_MAXCOUNT * 2;
  short addition = SUPLA_SCENE_PACK_MAXCOUNT / 5;
  short a = 0;

  for (a = 0; a < count + addition; a++) {
    supla_client_scene *scene = new supla_client_scene(a + 1);
    if (scene) {
      scene->set_change_indicator(
          new supla_client_scene_change_indicator(true, a == 0));
      scenes.add(scene);
    }
  }

  EXPECT_EQ(scenes.count(), count + addition);

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&scenePack), Return(1)));
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(scenePack.count, count / 2);
  EXPECT_EQ(scenePack.total_left, count / 2 + addition);

  for (a = 0; a < count / 2; a++) {
    EXPECT_EQ(scenePack.items[a].Id, a + 1);
    EXPECT_EQ(scenePack.items[a].EOL, 0);
  }

  scenePack = {};

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&scenePack), Return(1)));
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(scenePack.count, count / 2);
  EXPECT_EQ(scenePack.total_left, addition);

  for (a = 0; a < count / 2; a++) {
    EXPECT_EQ(scenePack.items[a].Id, count / 2 + a + 1);
    EXPECT_EQ(scenePack.items[a].EOL, 0);
  }

  scenePack = {};

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&scenePack), Return(1)));
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(scenePack.count, addition);
  EXPECT_EQ(scenePack.total_left, 0);

  for (a = 0; a < addition; a++) {
    EXPECT_EQ(scenePack.items[a].Id, count + a + 1);
    EXPECT_EQ(scenePack.items[a].EOL, a == addition - 1 ? 1 : 0);
  }

  TSC_SuplaSceneStatePack statePack = {};

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);

  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&statePack), Return(1)));

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(statePack.count, 1);
  EXPECT_EQ(statePack.total_left, 0);

  EXPECT_EQ(statePack.items[0].SceneId, 1);
  EXPECT_EQ(statePack.items[0].EOL, 1);

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(1);
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_FALSE(scenes.update_remote());
}

TEST_F(ClientSceneRemoteUpdateTest, sceneUpdate) {
  SrpcAdapterMock srpcAdapter(NULL);

  TSC_SuplaScenePack scenePack = {};

  supla_client_scene_remote_updater remoteUpdater(&srpcAdapter);
  DObjectsMock scenes(&remoteUpdater);

  supla_client_scene *scene = new supla_client_scene(123);
  if (scene) {
    scene->set_change_indicator(
        new supla_client_scene_change_indicator(true, false));
    scene->set_alt_icon_id(345);
    scene->set_caption("My Scene");
    scene->set_user_icon_id(890);
    scene->set_location_id(56);
    scenes.add(scene);
  }

  EXPECT_EQ(scenes.count(), 1);

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&scenePack), Return(1)));
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(scenePack.count, 1);
  EXPECT_EQ(scenePack.total_left, 0);
  EXPECT_EQ(scenePack.items[0].Id, 123);
  EXPECT_EQ(scenePack.items[0].AltIcon, 345);
  EXPECT_EQ(scenePack.items[0].UserIcon, 890);
  EXPECT_EQ(scenePack.items[0].LocationId, 56);
  EXPECT_EQ(scenePack.items[0].EOL, 1);
  EXPECT_EQ(strncmp(scenePack.items[0].Caption, scene->get_caption(),
                    SUPLA_SCENE_CAPTION_MAXSIZE),
            0);
  EXPECT_EQ(scenePack.items[0].CaptionSize,
            strnlen(scene->get_caption(), SUPLA_SCENE_CAPTION_MAXSIZE) + 1);

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(1);
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_FALSE(scenes.update_remote());
}

TEST_F(ClientSceneRemoteUpdateTest, sceneAndStateUpdate) {
  SrpcAdapterMock srpcAdapter(NULL);

  TSC_SuplaScenePack scenePack = {};

  supla_client_scene_remote_updater remoteUpdater(&srpcAdapter);
  DObjectsMock scenes(&remoteUpdater);

  struct timeval now = {};
  gettimeofday(&now, NULL);
  now.tv_sec--;

  const char initiatorName[] = "iPhone Steve";

  supla_scene_state state(supla_caller(ctClient, 34, initiatorName), now, 6000);

  supla_client_scene *scene = new supla_client_scene(12356);
  if (scene) {
    scene->set_change_indicator(
        new supla_client_scene_change_indicator(true, true));
    scene->set_alt_icon_id(3455);
    scene->set_caption("My Scene!");
    scene->set_user_icon_id(8900);
    scene->set_location_id(560);
    scene->set_state(state);
    scenes.add(scene);
  }

  EXPECT_EQ(scenes.count(), 1);

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&scenePack), Return(1)));
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(scenePack.count, 1);
  EXPECT_EQ(scenePack.total_left, 0);
  EXPECT_EQ(scenePack.items[0].Id, 12356);
  EXPECT_EQ(scenePack.items[0].AltIcon, 3455);
  EXPECT_EQ(scenePack.items[0].UserIcon, 8900);
  EXPECT_EQ(scenePack.items[0].LocationId, 560);
  EXPECT_EQ(scenePack.items[0].EOL, 1);
  EXPECT_EQ(strncmp(scenePack.items[0].Caption, scene->get_caption(),
                    SUPLA_SCENE_CAPTION_MAXSIZE),
            0);
  EXPECT_EQ(scenePack.items[0].CaptionSize,
            strnlen(scene->get_caption(), SUPLA_SCENE_CAPTION_MAXSIZE) + 1);

  TSC_SuplaSceneStatePack statePack = {};

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&statePack), Return(1)));

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(statePack.count, 1);
  EXPECT_EQ(statePack.total_left, 0);
  EXPECT_EQ(statePack.items[0].EOL, 1);
  EXPECT_EQ(statePack.items[0].SceneId, 12356);
  EXPECT_EQ(statePack.items[0].InitiatorId, 34);
  EXPECT_GE(statePack.items[0].MillisecondsFromStart, 1000);
  EXPECT_LE(statePack.items[0].MillisecondsFromStart, 1200);
  EXPECT_LE(statePack.items[0].MillisecondsLeft, 5000);
  EXPECT_GE(statePack.items[0].MillisecondsLeft, 4800);
  EXPECT_EQ(strncmp(statePack.items[0].InitiatorName,
                    scene->get_state().get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE),
            0);
  EXPECT_EQ(statePack.items[0].InitiatorNameSize,
            strnlen(scene->get_state().get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE) +
                1);

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(1);
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_FALSE(scenes.update_remote());
}

TEST_F(ClientSceneRemoteUpdateTest, stateUpdate) {
  SrpcAdapterMock srpcAdapter(NULL);

  supla_client_scene_remote_updater remoteUpdater(&srpcAdapter);
  DObjectsMock scenes(&remoteUpdater);

  char initiatorName[] = "iPhone Elon";

  struct timeval now = {};
  gettimeofday(&now, NULL);
  now.tv_sec -= 5;
  supla_scene_state state(supla_caller(ctClient, 3467, initiatorName), now,
                          6200);

  supla_client_scene *scene = new supla_client_scene(22356);
  if (scene) {
    scene->set_change_indicator(
        new supla_client_scene_change_indicator(false, true));
    scene->set_alt_icon_id(3455);
    scene->set_state(state);

    scenes.add(scene);
  }

  TSC_SuplaSceneStatePack statePack = {};

  EXPECT_CALL(srpcAdapter, get_proto_version)
      .Times(2)
      .WillRepeatedly(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&statePack), Return(1)));

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(statePack.count, 1);
  EXPECT_EQ(statePack.total_left, 0);
  EXPECT_EQ(statePack.items[0].EOL, 1);
  EXPECT_EQ(statePack.items[0].SceneId, 22356);
  EXPECT_EQ(statePack.items[0].InitiatorId, 3467);
  EXPECT_GE(statePack.items[0].MillisecondsFromStart, 5000);
  EXPECT_LE(statePack.items[0].MillisecondsFromStart, 5200);
  EXPECT_LE(statePack.items[0].MillisecondsLeft, 1200);
  EXPECT_GE(statePack.items[0].MillisecondsLeft, 1000);
  EXPECT_EQ(strncmp(statePack.items[0].InitiatorName,
                    scene->get_state().get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE),
            0);
  EXPECT_EQ(statePack.items[0].InitiatorNameSize,
            strnlen(scene->get_state().get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE) +
                1);

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(1);
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_FALSE(scenes.update_remote());
}

} /* namespace testing */
