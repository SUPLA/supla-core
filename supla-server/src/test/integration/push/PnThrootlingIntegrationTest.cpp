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

#include "PnThrootlingIntegrationTest.h"

#include "doubles/push/PnThrottlingStub.h"

namespace testing {

PnThrootlingIntegrationTest::PnThrootlingIntegrationTest()
    : IntegrationTest(), Test() {}

PnThrootlingIntegrationTest::~PnThrootlingIntegrationTest() {}

void PnThrootlingIntegrationTest::SetUp() {
  Test::SetUp();
  initTestDatabase();
}

TEST_F(PnThrootlingIntegrationTest, defaultWindowTime) {
  PnThrootlingStub t;
  EXPECT_EQ(t.get_time_window_sec(), 3600);
}

TEST_F(PnThrootlingIntegrationTest, userId2) {
  bool first_time_exceeded = false;
  unsigned int limit = 0;
  bool delivery_possible = false;

  PnThrootlingStub t;
  t.set_time_window_sec(2);

  for (unsigned int a = 1; a < 100; a++) {
    delivery_possible = t.is_delivery_possible(2, &first_time_exceeded, &limit);

    EXPECT_EQ(t.get_count(2), a);
    EXPECT_EQ(limit, 20);
    if (a <= limit) {
      EXPECT_TRUE(delivery_possible);
    } else {
      EXPECT_FALSE(delivery_possible);
    }

    if (a == limit + 1) {
      EXPECT_TRUE(first_time_exceeded);
    } else {
      EXPECT_FALSE(first_time_exceeded);
    }
  }

  usleep(2000000);
  delivery_possible = t.is_delivery_possible(2, &first_time_exceeded, &limit);

  EXPECT_TRUE(delivery_possible);
  EXPECT_EQ(limit, 20);
  EXPECT_FALSE(first_time_exceeded);
  EXPECT_EQ(t.get_user_count(), 1);
}

TEST_F(PnThrootlingIntegrationTest, changeLimits) {
  bool first_time_exceeded = false;
  unsigned int limit = 0;

  PnThrootlingStub t;
  t.set_time_window_sec(2);
  EXPECT_EQ(t.get_user_count(), 0);
  t.is_delivery_possible(2, &first_time_exceeded, &limit);

  runSqlScript("ChangePnLimits.sql");

  for (unsigned int a = 1; a < 30; a++) {
    t.is_delivery_possible(2, &first_time_exceeded, &limit);
    EXPECT_EQ(limit, 20);
  }

  usleep(2000000);
  t.is_delivery_possible(2, &first_time_exceeded, &limit);
  EXPECT_EQ(limit, 5);
  EXPECT_EQ(t.get_user_count(), 1);
}

TEST_F(PnThrootlingIntegrationTest, nonExistentUser) {
  PnThrootlingStub t;
  EXPECT_EQ(t.get_user_count(), 0);
  bool first_time_exceeded = false;
  unsigned int limit = 0;

  EXPECT_FALSE(t.is_delivery_possible(5, &first_time_exceeded, &limit));
  EXPECT_TRUE(first_time_exceeded);
  EXPECT_EQ(limit, 0);
  EXPECT_EQ(t.get_user_count(), 1);
}

TEST_F(PnThrootlingIntegrationTest, multipleUsers) {
  runSqlScript("AddUsersWithDifferentPnLimits.sql");
  PnThrootlingStub t;

  bool first_time_exceeded = false;
  unsigned int limit = 0;

  EXPECT_TRUE(t.is_delivery_possible(2, &first_time_exceeded, &limit));
  EXPECT_EQ(t.get_count(2), 1);
  EXPECT_EQ(t.get_user_count(), 1);
  EXPECT_EQ(limit, 20);
  EXPECT_FALSE(first_time_exceeded);

  EXPECT_TRUE(t.is_delivery_possible(48, &first_time_exceeded, &limit));
  EXPECT_EQ(t.get_count(2), 1);
  EXPECT_EQ(t.get_user_count(), 2);
  EXPECT_EQ(limit, 10);
  EXPECT_FALSE(first_time_exceeded);

  EXPECT_TRUE(t.is_delivery_possible(53, &first_time_exceeded, &limit));
  EXPECT_EQ(t.get_count(2), 1);
  EXPECT_EQ(t.get_user_count(), 3);
  EXPECT_EQ(limit, 20);
  EXPECT_FALSE(first_time_exceeded);

  EXPECT_TRUE(t.is_delivery_possible(117, &first_time_exceeded, &limit));
  EXPECT_EQ(t.get_count(2), 1);
  EXPECT_EQ(t.get_user_count(), 4);
  EXPECT_EQ(limit, 30);
  EXPECT_FALSE(first_time_exceeded);

  EXPECT_FALSE(t.is_delivery_possible(121, &first_time_exceeded, &limit));
  EXPECT_EQ(t.get_count(2), 1);
  EXPECT_EQ(t.get_user_count(), 5);
  EXPECT_EQ(limit, 0);
  EXPECT_TRUE(first_time_exceeded);
}

} /* namespace testing */
