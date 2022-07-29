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

#include "scene/SceneStateTest.h"

#include "scene/scene_state.h"

namespace testing {

SceneStateTest::SceneStateTest() : Test() {}

SceneStateTest::~SceneStateTest() {}

TEST_F(SceneStateTest, emptyConstructor) {
  supla_scene_state state;
  EXPECT_TRUE(state.get_caller() == supla_caller());
  EXPECT_FALSE(state.is_during_execution());
  EXPECT_EQ(state.get_initiator_id(), 0);
  EXPECT_TRUE(state.get_initiator_name() == NULL);

  struct timeval zero_time = {};
  struct timeval time = state.get_started_at();
  EXPECT_EQ(memcmp(&time, &zero_time, sizeof(struct timeval)), 0);

  time = state.get_ending_at();
  EXPECT_EQ(memcmp(&time, &zero_time, sizeof(struct timeval)), 0);

  EXPECT_EQ(state.get_milliseconds_from_start(), 0U);
  EXPECT_EQ(state.get_milliseconds_left(), 0U);
}

TEST_F(SceneStateTest, caller) {
  supla_scene_state state(supla_caller(ctScene, 10), {}, 0);
  EXPECT_TRUE(state.get_caller() == supla_caller(ctScene, 10));
}

TEST_F(SceneStateTest, initiatorId) {
  {
    supla_scene_state state(supla_caller(ctScene, 10), {}, 0);
    EXPECT_EQ(state.get_initiator_id(), 0);
  }

  {
    supla_scene_state state(supla_caller(ctClient, 15), {}, 0);
    EXPECT_EQ(state.get_initiator_id(), 15);
  }
}

TEST_F(SceneStateTest, initiatorName) {
  {
    supla_scene_state state(supla_caller(ctScene, 10), {}, 0);
    EXPECT_TRUE(state.get_initiator_name() == NULL);
  }

  {
    const char initiatorName[] = "iPhone Elon";
    supla_scene_state state(supla_caller(ctClient, 10, initiatorName), {}, 0);
    EXPECT_TRUE(state.get_initiator_name() != NULL);
    EXPECT_EQ(strncmp(state.get_initiator_name(), initiatorName,
                      sizeof(initiatorName)),
              0);
    EXPECT_EQ(strncmp(state.get_initiator_name(), initiatorName,
                      sizeof(initiatorName)),
              0);
  }
}

TEST_F(SceneStateTest, checkIfItIsDuringTheExecution) {
  {
    supla_scene_state state(supla_caller(), {}, 0);
    EXPECT_FALSE(state.is_during_execution());
  }

  struct timeval now = {};
  gettimeofday(&now, NULL);

  {
    supla_scene_state state(supla_caller(), now, 0);
    EXPECT_FALSE(state.is_during_execution());
  }

  {
    supla_scene_state state(supla_caller(), now, 100);
    EXPECT_TRUE(state.is_during_execution());

    usleep(200000);
    EXPECT_FALSE(state.is_during_execution());
  }

  now.tv_sec++;

  {
    supla_scene_state state(supla_caller(), now, 2000);
    EXPECT_FALSE(state.is_during_execution());

    usleep(1000000);
    EXPECT_TRUE(state.is_during_execution());
  }
}

TEST_F(SceneStateTest, startedAt) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  supla_scene_state state(supla_caller(), now, 0);
  struct timeval tv = state.get_started_at();

  EXPECT_EQ(now.tv_sec, tv.tv_sec);
  EXPECT_EQ(now.tv_usec, tv.tv_usec);
}

TEST_F(SceneStateTest, endingAt) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  supla_scene_state state(supla_caller(), now, 123);
  struct timeval tv = state.get_ending_at();

  unsigned long long now_usec = now.tv_sec * 1000000 + now.tv_usec;
  unsigned long long tv_usec = tv.tv_sec * 1000000 + tv.tv_usec;

  EXPECT_EQ(tv_usec - now_usec, 123000U);
}

TEST_F(SceneStateTest, millisecondsFromStart) {
  {
    supla_scene_state state(supla_caller(), {}, 0);
    EXPECT_EQ(state.get_milliseconds_from_start(), 0U);
  }

  struct timeval now = {};
  gettimeofday(&now, NULL);

  now.tv_sec++;

  {
    supla_scene_state state(supla_caller(), now, 0);
    EXPECT_EQ(state.get_milliseconds_from_start(), 0U);
  }

  now.tv_sec -= 2;

  {
    supla_scene_state state(supla_caller(), now, 0);
    EXPECT_GE(state.get_milliseconds_from_start(), 1000U);
  }
}

TEST_F(SceneStateTest, millisecondsLeft) {
  {
    supla_scene_state state(supla_caller(), {}, 0);
    EXPECT_EQ(state.get_milliseconds_left(), 0U);
  }

  struct timeval now = {};
  gettimeofday(&now, NULL);

  {
    supla_scene_state state(supla_caller(), now, 0);
    EXPECT_EQ(state.get_milliseconds_left(), 0U);
  }

  {
    supla_scene_state state(supla_caller(), now, 500);
    EXPECT_GE(state.get_milliseconds_left(), 100U);
    EXPECT_LE(state.get_milliseconds_left(), 500U);
  }

  now.tv_sec++;

  {
    supla_scene_state state(supla_caller(), now, 500);
    EXPECT_GE(state.get_milliseconds_left(), 0U);
  }
}

TEST_F(SceneStateTest, copy) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  const char initiatorName[] = "iPhone Steve";
  supla_scene_state state1(supla_caller(ctClient, 55, initiatorName), now, 500);

  supla_scene_state state2 = state1;
  EXPECT_TRUE(state1.get_caller() == state2.get_caller());
  EXPECT_TRUE(state1.is_during_execution() == state2.is_during_execution());
  EXPECT_TRUE(state1.get_initiator_id() == state2.get_initiator_id());
  EXPECT_EQ(state1.get_started_at().tv_sec, state2.get_started_at().tv_sec);
  EXPECT_EQ(state1.get_started_at().tv_usec, state2.get_started_at().tv_usec);
  EXPECT_EQ(state1.get_ending_at().tv_sec, state2.get_ending_at().tv_sec);
  EXPECT_EQ(state1.get_ending_at().tv_usec, state2.get_ending_at().tv_usec);
  EXPECT_FALSE(state1.get_initiator_name() == NULL);
  EXPECT_FALSE(state2.get_initiator_name() == NULL);
  EXPECT_NE(state1.get_initiator_name(), state2.get_initiator_name());
  EXPECT_EQ(strncmp(state2.get_initiator_name(), initiatorName,
                    sizeof(initiatorName)),
            0);
}

TEST_F(SceneStateTest, compare) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  const char initiatorName[] = "iPhone Steve";
  supla_scene_state state1(supla_caller(ctClient, 55, initiatorName), now, 500);

  supla_scene_state state2(supla_caller(ctClient, 55, initiatorName), now, 501);

  struct timeval _now = now;
  _now.tv_sec++;

  supla_scene_state state3(supla_caller(ctClient, 55, initiatorName), _now,
                           500);

  _now = now;
  _now.tv_usec++;

  supla_scene_state state4(supla_caller(ctClient, 55, initiatorName), _now,
                           500);

  supla_scene_state state5(supla_caller(ctClient, 56, initiatorName), now, 500);

  supla_scene_state state6 = state1;

  EXPECT_FALSE(state1 == state2);
  EXPECT_TRUE(state1 != state2);

  EXPECT_FALSE(state1 == state3);
  EXPECT_TRUE(state1 != state3);

  EXPECT_FALSE(state1 == state4);
  EXPECT_TRUE(state1 != state4);

  EXPECT_FALSE(state1 == state5);
  EXPECT_TRUE(state1 != state5);

  EXPECT_TRUE(state1 == state6);
  EXPECT_FALSE(state1 != state6);
}

TEST_F(SceneStateTest, convert) {
  struct timeval now = {};
  gettimeofday(&now, NULL);
  now.tv_sec--;
  supla_scene_state state(supla_caller(ctClient, 55, "Windows Phone Bill"), now,
                          1500);

  TSC_SuplaSceneState sc_scene_state = {};
  state.convert(&sc_scene_state);

  EXPECT_EQ(sc_scene_state.EOL, 0);
  EXPECT_EQ(sc_scene_state.SceneId, 0);
  EXPECT_EQ(sc_scene_state.InitiatorId, 55);
  EXPECT_GE(sc_scene_state.MillisecondsFromStart, 1000U);
  EXPECT_LT(sc_scene_state.MillisecondsFromStart, 1200U);
  EXPECT_GE(sc_scene_state.MillisecondsLeft, 300U);
  EXPECT_LE(sc_scene_state.MillisecondsLeft, 500U);

  EXPECT_EQ(
      sc_scene_state.InitiatorNameSize,
      strnlen(state.get_initiator_name(), SUPLA_INITIATOR_NAME_MAXSIZE) + 1);

  EXPECT_EQ(strncmp(sc_scene_state.InitiatorName, state.get_initiator_name(),
                    SUPLA_INITIATOR_NAME_MAXSIZE + 1),
            0);
}

} /* namespace testing */
