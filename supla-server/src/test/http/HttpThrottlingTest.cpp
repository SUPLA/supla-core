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

#include "http/HttpThrottlingTest.h"

#include "proto.h"

namespace testing {

using std::string;

void HttpThrottlingTest::SetUp(void) {
  ON_CALL(thr, get_reset_time_us).WillByDefault(Return(2000000));
}

TEST_F(HttpThrottlingTest, defaults) {
  EXPECT_CALL(thr, get_reset_time_us).WillOnce([this](int func) {
    return thr.supla_http_throttling::get_reset_time_us(func);
  });

  EXPECT_EQ(thr.get_size(), 0);
  EXPECT_EQ(thr.get_default_delay_time(0), 1500000);
  EXPECT_EQ(thr.get_delay_time_over_threadshold(0), 10000000);
  EXPECT_EQ(thr.get_reset_time_us(0), 10000000);
  EXPECT_EQ(thr.get_counter_threadshold(0), 5);
  EXPECT_EQ(thr.get_counter_threadshold(SUPLA_CHANNELFNC_DIMMER), 20);
  EXPECT_EQ(thr.get_counter_threadshold(SUPLA_CHANNELFNC_RGBLIGHTING), 20);
  EXPECT_EQ(thr.get_counter_threadshold(SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING),
            20);
}

TEST_F(HttpThrottlingTest, crossingTheThreadshold) {
  for (unsigned int a = 0; a < thr.get_counter_threadshold(0) - 1; a++) {
    EXPECT_EQ(thr.get_delay_time(1, 0), thr.get_default_delay_time(0));
  }

  for (unsigned int a = 0; a < thr.get_counter_threadshold(0) - 1; a++) {
    usleep(thr.get_reset_time_us(0) - 200000);
    EXPECT_EQ(thr.get_delay_time(1, 0), thr.get_delay_time_over_threadshold(0));
    EXPECT_EQ(thr.get_delay_time(2, 0), thr.get_default_delay_time(0));
  }

  usleep(thr.get_reset_time_us(0));

  EXPECT_EQ(thr.get_delay_time(1, 0), thr.get_default_delay_time(0));
  EXPECT_EQ(thr.get_delay_time(2, 0), thr.get_default_delay_time(0));

  for (unsigned int a = 0; a < thr.get_counter_threadshold(0) - 2; a++) {
    EXPECT_EQ(thr.get_delay_time(1, 0), thr.get_default_delay_time(0));
  }

  EXPECT_EQ(thr.get_delay_time(1, 0), thr.get_delay_time_over_threadshold(0));
}

TEST_F(HttpThrottlingTest, size) {
  unsigned int a = 0;
  for (a = 1; a <= 10; a++) {
    thr.get_delay_time(a, 0);
    EXPECT_EQ(thr.get_size(), a);
  }

  for (a = 1; a <= 10; a++) {
    thr.get_delay_time(a, 0);
    EXPECT_EQ(thr.get_size(), 10);
  }

  for (a = 11; a <= 20; a++) {
    thr.get_delay_time(a, 0);
    EXPECT_EQ(thr.get_size(), a);
  }
}

}  // namespace testing
