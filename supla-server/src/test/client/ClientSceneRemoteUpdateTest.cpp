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

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(2).WillOnce(Return(18));
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

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(2).WillOnce(Return(18));
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

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(2).WillOnce(Return(18));
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

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(2).WillOnce(Return(18));
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


} /* namespace testing */
