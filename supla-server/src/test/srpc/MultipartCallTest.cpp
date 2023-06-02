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

#include "MultipartCallTest.h"

#include "TestHelper.h"
#include "srpc/multipart_call.h"

namespace testing {

MultipartCallTest::MultipartCallTest() : Test() {}

MultipartCallTest::~MultipartCallTest() {}

TEST_F(MultipartCallTest, expirationTest) {
  supla_multipart_call call1(1, 0);
  EXPECT_TRUE(call1.is_expired());

  supla_multipart_call call2(1, 500);
  EXPECT_FALSE(call2.is_expired());
  usleep(200000);
  EXPECT_FALSE(call2.is_expired());
  usleep(300000);
  EXPECT_TRUE(call2.is_expired());
}

TEST_F(MultipartCallTest, callIdGetter) {
  supla_multipart_call call(123, 0);
  EXPECT_EQ(call.get_call_id(), 123);
}

TEST_F(MultipartCallTest, free) {
  // The result of this test is only applicable in cooperation with valgrind

  char p1[1024] = {};
  char p2[512] = {};
  char p3[1] = {};

  supla_multipart_call call(1, 0);
  call.part_push(p1, sizeof(p1));
  call.part_push(p2, sizeof(p2));
  call.part_push(p3, sizeof(p3));
}

TEST_F(MultipartCallTest, pushPop) {
  char p1[1024] = {};
  char p2[512] = {};

  TestHelper::randomize(p1, sizeof(p1));
  TestHelper::randomize(p2, sizeof(p2));

  supla_multipart_call call(0, 0);
  call.part_push(p1, sizeof(p1));
  call.part_push(p2, sizeof(p2));

  size_t data_size = 0;
  char *data = call.part_pop(&data_size);
  ASSERT_NE(data, nullptr);
  EXPECT_NE(data, p1);
  EXPECT_EQ(data_size, sizeof(p1));
  EXPECT_EQ(memcmp(data, p1, sizeof(p1)), 0);
  delete[] data;

  data_size = 0;
  data = call.part_pop(&data_size);
  ASSERT_NE(data, nullptr);
  EXPECT_NE(data, p2);
  EXPECT_EQ(data_size, sizeof(p2));
  EXPECT_EQ(memcmp(data, p2, sizeof(p2)), 0);
  delete[] data;

  EXPECT_EQ(call.part_pop(&data_size), nullptr);
}

}  // namespace testing
