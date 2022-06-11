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

#include <integration/client/ClientSceneIntegrationTest.h>

#include "client/client_scene_dao.h"
#include "client/client_scene_remote_updater.h"
#include "client/client_scenes.h"
#include "doubles/SrpcAdapterMock.h"

namespace testing {

ClientSceneIntegrationTest::ClientSceneIntegrationTest()
    : IntegrationTest(), Test() {}

ClientSceneIntegrationTest::~ClientSceneIntegrationTest() {}

void ClientSceneIntegrationTest::SetUp() {
  initTestDatabase();
  Test::SetUp();
}

TEST_F(ClientSceneIntegrationTest, loadAndUpdateRemote) {
  SrpcAdapterMock srpcAdapter(NULL);

  TSC_SuplaScenePack scenePack = {};

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(2).WillOnce(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&scenePack), Return(1)));
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  supla_client_scene_remote_updater remoteUpdater(&srpcAdapter);

  supla_client_scene_dao dao;
  supla_client_scenes scenes(&remoteUpdater, &dao);

  scenes.load(2, 1);
  EXPECT_EQ(scenes.count(), 6);
  EXPECT_EQ(scenePack.count, 0);

  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(scenePack.count, 6);
  EXPECT_EQ(scenePack.total_left, 0);

  for (int a = 0; a < scenePack.count; a++) {
    char caption[10];
    snprintf(caption, sizeof(caption), "Scene #%i", a + 1);
    EXPECT_EQ(scenePack.items[a].Id, a + 1);
    EXPECT_EQ(strncmp(scenePack.items[a].Caption, caption,
                      SUPLA_SCENE_CAPTION_MAXSIZE),
              0);
    EXPECT_EQ(scenePack.items[a].CaptionSize,
              strnlen(caption, sizeof(caption)) + 1);
    EXPECT_EQ(scenePack.items[a].EOL, a == scenePack.count - 1 ? 1 : 0);
  }

  TSC_SuplaSceneStatePack statePack = {};

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(2).WillOnce(Return(18));
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update)
      .Times(1)
      .WillOnce(DoAll(SaveArgPointee<0>(&statePack), Return(1)));

  EXPECT_EQ(statePack.count, 0);
  EXPECT_TRUE(scenes.update_remote());

  EXPECT_EQ(statePack.count, 6);
  EXPECT_EQ(statePack.total_left, 0);

  for (int a = 0; a < statePack.count; a++) {
    EXPECT_EQ(statePack.items[a].SceneId, a + 1);
    EXPECT_EQ(statePack.items[a].EOL, a == statePack.count - 1 ? 1 : 0);
  }

  EXPECT_CALL(srpcAdapter, get_proto_version).Times(1);
  EXPECT_CALL(srpcAdapter, sc_async_scene_pack_update).Times(0);
  EXPECT_CALL(srpcAdapter, sc_async_scene_state_pack_update).Times(0);

  EXPECT_FALSE(scenes.update_remote());
}

} /* namespace testing */
