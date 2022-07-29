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

#include "asynctask/AsyncTaskStateTest.h"

#include "asynctask/asynctask_state.h"

namespace testing {

AsyncTaskStateTest::AsyncTaskStateTest(void) {}

AsyncTaskStateTest::~AsyncTaskStateTest(void) {}

TEST_F(AsyncTaskStateTest, constructor) {
  supla_asynctask_state s1;
  EXPECT_EQ(s1, supla_asynctask_state::INIT);
  s1 = supla_asynctask_state::EXECUTING;

  supla_asynctask_state s2(s1);
  EXPECT_EQ(s2, supla_asynctask_state::EXECUTING);

  supla_asynctask_state s3(supla_asynctask_state::PICKED);
  EXPECT_EQ(s3, supla_asynctask_state::PICKED);
}

TEST_F(AsyncTaskStateTest, setterGetter) {
  supla_asynctask_state state;
  EXPECT_EQ(state, supla_asynctask_state::INIT);
  state.set_state(supla_asynctask_state::SUCCESS);
  EXPECT_EQ(state, supla_asynctask_state::SUCCESS);
}

TEST_F(AsyncTaskStateTest, assignmentOperator) {
  supla_asynctask_state state;
  EXPECT_EQ(state, supla_asynctask_state::INIT);
  state = supla_asynctask_state::WAITING;
  EXPECT_EQ(state, supla_asynctask_state::WAITING);

  supla_asynctask_state::_state_e s = supla_asynctask_state::PICKED;
  s = state;
  EXPECT_EQ(s, supla_asynctask_state::WAITING);
}

TEST_F(AsyncTaskStateTest, conversionOperator) {
  supla_asynctask_state state;
  supla_asynctask_state::_state_e s = supla_asynctask_state::PICKED;
  s = state;
  EXPECT_EQ(s, supla_asynctask_state::INIT);
}

TEST_F(AsyncTaskStateTest, comparsionOperator) {
  supla_asynctask_state state1;
  supla_asynctask_state state2;

  EXPECT_TRUE(state1 == state2);
  EXPECT_FALSE(state1 != state2);

  state2 = supla_asynctask_state::PICKED;

  EXPECT_FALSE(state1 == state2);
  EXPECT_TRUE(state1 != state2);

  state1 = supla_asynctask_state::PICKED;

  EXPECT_TRUE(state1 == state2);
  EXPECT_FALSE(state1 != state2);

  supla_asynctask_state::_state_e s = supla_asynctask_state::CANCELED;
  EXPECT_FALSE(state1 == s);
  EXPECT_TRUE(state1 != s);

  state2 = s;
  EXPECT_FALSE(state2 != s);
  EXPECT_TRUE(state2 == s);
}

}  // namespace testing
