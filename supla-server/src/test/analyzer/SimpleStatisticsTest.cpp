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

#include "test/analyzer/SimpleStatisticsTest.h"

#include <math.h>

namespace testing {

TEST_F(SimpleStatisticsTest, min) {
  EXPECT_TRUE(isnan(stat.get_min()));
  EXPECT_EQ(stat.get_sample_count(), 0);
  stat.add_sample(100);
  EXPECT_EQ(stat.get_min(), 100);
  stat.add_sample(10);
  EXPECT_EQ(stat.get_min(), 10);
  EXPECT_EQ(stat.get_sample_count(), 2);
  stat.reset();
  EXPECT_EQ(stat.get_sample_count(), 0);
  EXPECT_TRUE(isnan(stat.get_min()));
}

TEST_F(SimpleStatisticsTest, max) {
  EXPECT_TRUE(isnan(stat.get_max()));
  EXPECT_EQ(stat.get_sample_count(), 0);
  stat.add_sample(100);
  EXPECT_EQ(stat.get_max(), 100);
  stat.add_sample(110);
  EXPECT_EQ(stat.get_max(), 110);
  EXPECT_EQ(stat.get_sample_count(), 2);
  stat.reset();
  EXPECT_EQ(stat.get_sample_count(), 0);
  EXPECT_TRUE(isnan(stat.get_max()));
}

TEST_F(SimpleStatisticsTest, avg) {
  EXPECT_TRUE(isnan(stat.get_avg()));
  EXPECT_EQ(stat.get_sample_count(), 0);
  stat.add_sample(100);
  EXPECT_EQ(stat.get_avg(), 100);
  stat.add_sample(200);
  EXPECT_EQ(stat.get_avg(), 150);
  EXPECT_EQ(stat.get_sample_count(), 2);
  stat.reset();
  EXPECT_EQ(stat.get_sample_count(), 0);
  EXPECT_TRUE(isnan(stat.get_avg()));
}

TEST_F(SimpleStatisticsTest, firstAndLast) {
  EXPECT_TRUE(isnan(stat.get_first()));
  EXPECT_TRUE(isnan(stat.get_last()));
  EXPECT_EQ(stat.get_sample_count(), 0);
  stat.add_sample(100);
  EXPECT_EQ(stat.get_first(), 100);
  EXPECT_EQ(stat.get_last(), 100);
  EXPECT_EQ(stat.get_sample_count(), 1);
  stat.add_sample(101);
  EXPECT_EQ(stat.get_first(), 100);
  EXPECT_EQ(stat.get_last(), 101);
  EXPECT_EQ(stat.get_sample_count(), 2);
  stat.add_sample(103);
  EXPECT_EQ(stat.get_first(), 100);
  EXPECT_EQ(stat.get_last(), 103);
  EXPECT_EQ(stat.get_sample_count(), 3);
  stat.reset();
  EXPECT_EQ(stat.get_sample_count(), 0);
  EXPECT_TRUE(isnan(stat.get_first()));
  EXPECT_TRUE(isnan(stat.get_last()));
}

TEST_F(SimpleStatisticsTest, totalTime) {
  EXPECT_EQ(stat.get_total_time_msec(), 0);
  stat.reset();
  usleep(100000);
  EXPECT_EQ(stat.get_total_time_msec(), 0);
  stat.reset();
  stat.add_sample(123);
  EXPECT_EQ(stat.get_total_time_msec(), 0);
  stat.reset();
  stat.add_sample(123);
  usleep(50000);
  stat.add_sample(124);
  usleep(60000);
  EXPECT_GE(stat.get_total_time_msec(), 110);
  EXPECT_LT(stat.get_total_time_msec(), 120);
  stat.reset();
}

TEST_F(SimpleStatisticsTest, reset) {
  stat.add_sample(5);
  EXPECT_EQ(stat.get_sample_count(), 1);
  EXPECT_EQ(stat.get_first(), 5);
  EXPECT_EQ(stat.get_min(), 5);
  EXPECT_EQ(stat.get_max(), 5);
  EXPECT_EQ(stat.get_avg(), 5);
  EXPECT_EQ(stat.get_last(), 5);
  stat.reset();
  EXPECT_EQ(stat.get_sample_count(), 0);
  EXPECT_TRUE(isnan(stat.get_first()));
  EXPECT_TRUE(isnan(stat.get_min()));
  EXPECT_TRUE(isnan(stat.get_max()));
  EXPECT_TRUE(isnan(stat.get_avg()));
  EXPECT_TRUE(isnan(stat.get_last()));
}

TEST_F(SimpleStatisticsTest, nan) {
  stat.add_sample(NAN);
  stat.add_sample(10);
  stat.add_sample(NAN);
  stat.add_sample(5);
  stat.add_sample(NAN);
  stat.add_sample(15);
  stat.add_sample(NAN);
  stat.add_sample(8);
  stat.add_sample(NAN);

  EXPECT_EQ(stat.get_first(), 10);
  EXPECT_EQ(stat.get_min(), 5);
  EXPECT_EQ(stat.get_max(), 15);
  EXPECT_EQ(stat.get_last(), 8);
  EXPECT_DOUBLE_EQ(stat.get_avg(), 9.5);
  EXPECT_EQ(stat.get_sample_count(), 9);
  EXPECT_EQ(stat.get_non_nan_sample_count(), 4);
}

}  // namespace testing
