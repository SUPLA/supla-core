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

#include "client/ClientSceneChangeIndicatorTest.h"

#include "client/client_scene_change_indicator.h"

namespace testing {

ClientSceneChangeIndicatorTest::ClientSceneChangeIndicatorTest() : Test() {}

ClientSceneChangeIndicatorTest::~ClientSceneChangeIndicatorTest() {}

TEST_F(ClientSceneChangeIndicatorTest, emptyConstructor) {
  supla_client_scene_change_indicator ind;
  EXPECT_TRUE(ind.is_changed());
  EXPECT_TRUE(ind.is_scene_changed());
  EXPECT_TRUE(ind.is_state_changed());
}

TEST_F(ClientSceneChangeIndicatorTest, constructorWithOneParameter) {
  supla_client_scene_change_indicator ind1(true);
  EXPECT_TRUE(ind1.is_changed());
  EXPECT_TRUE(ind1.is_scene_changed());
  EXPECT_TRUE(ind1.is_state_changed());

  supla_client_scene_change_indicator ind2(false);
  EXPECT_FALSE(ind2.is_changed());
  EXPECT_FALSE(ind2.is_scene_changed());
  EXPECT_FALSE(ind2.is_state_changed());
}

TEST_F(ClientSceneChangeIndicatorTest, constructorWithTwoParameters) {
  supla_client_scene_change_indicator ind1(false, false);

  EXPECT_FALSE(ind1.is_changed());
  EXPECT_FALSE(ind1.is_scene_changed());
  EXPECT_FALSE(ind1.is_state_changed());

  supla_client_scene_change_indicator ind2(true, false);

  EXPECT_TRUE(ind2.is_changed());
  EXPECT_TRUE(ind2.is_scene_changed());
  EXPECT_FALSE(ind2.is_state_changed());

  supla_client_scene_change_indicator ind3(true, true);

  EXPECT_TRUE(ind3.is_changed());
  EXPECT_TRUE(ind3.is_scene_changed());
  EXPECT_TRUE(ind3.is_state_changed());

  supla_client_scene_change_indicator ind4(false, true);

  EXPECT_TRUE(ind4.is_changed());
  EXPECT_FALSE(ind4.is_scene_changed());
  EXPECT_TRUE(ind4.is_state_changed());
}

TEST_F(ClientSceneChangeIndicatorTest, compare) {
  supla_client_scene_change_indicator ind1(false, false);
  supla_client_scene_change_indicator ind2(false, true);
  supla_client_scene_change_indicator ind3(true, true);
  supla_client_scene_change_indicator ind4(true, false);

  supla_client_scene_change_indicator ind1_bis = ind1;
  supla_client_scene_change_indicator ind2_bis = ind2;
  supla_client_scene_change_indicator ind3_bis = ind3;
  supla_client_scene_change_indicator ind4_bis = ind4;

  EXPECT_FALSE(ind1 == ind2);
  EXPECT_TRUE(ind1 != ind2);

  EXPECT_FALSE(ind1 == ind3);
  EXPECT_TRUE(ind1 != ind3);

  EXPECT_FALSE(ind1 == ind4);
  EXPECT_TRUE(ind1 != ind4);

  EXPECT_FALSE(ind2 == ind3);
  EXPECT_TRUE(ind2 != ind3);

  EXPECT_FALSE(ind2 == ind4);
  EXPECT_TRUE(ind2 != ind4);

  EXPECT_FALSE(ind3 == ind4);
  EXPECT_TRUE(ind3 != ind4);

  EXPECT_TRUE(ind1 == ind1_bis);
  EXPECT_FALSE(ind1 != ind1_bis);

  EXPECT_TRUE(ind2 == ind2_bis);
  EXPECT_FALSE(ind2 != ind2_bis);

  EXPECT_TRUE(ind3 == ind3_bis);
  EXPECT_FALSE(ind3 != ind3_bis);

  EXPECT_TRUE(ind4 == ind4_bis);
  EXPECT_FALSE(ind4 != ind4_bis);
}

} /* namespace testing */
