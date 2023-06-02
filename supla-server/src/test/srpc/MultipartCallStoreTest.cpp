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

#include "MultipartCallStoreTest.h"

#include "srpc/multipart_call_store.h"

namespace testing {

MultipartCallStoreTest::MultipartCallStoreTest() : Test() {}

MultipartCallStoreTest::~MultipartCallStoreTest() {}

TEST_F(MultipartCallStoreTest, addGetRelease) {
  supla_multipart_call_store store;
  supla_multipart_call *c1 = new supla_multipart_call(1, 1000);
  supla_multipart_call *c2a = new supla_multipart_call(2, 1000);
  supla_multipart_call *c2b = new supla_multipart_call(2, 1000);
  supla_multipart_call *c3 = new supla_multipart_call(3, 1000);
  store.add(c1);
  store.add(c2a);
  store.add(c2b);
  store.add(c3);

  EXPECT_EQ(store.size(), 3);
  EXPECT_EQ(store.get(1), c1);
  EXPECT_EQ(store.get(2), c2b);
  EXPECT_EQ(store.get(3), c3);

  EXPECT_EQ(store.size(), 3);
  store.release(1);
  EXPECT_EQ(store.size(), 2);
  store.release(2);
  EXPECT_EQ(store.size(), 1);
  store.release(3);
  EXPECT_EQ(store.size(), 0);
}

TEST_F(MultipartCallStoreTest, detach) {
  supla_multipart_call_store store;
  supla_multipart_call *c = new supla_multipart_call(1, 1000);
  store.add(c);

  EXPECT_EQ(store.size(), 1);
  store.detach(c);
  delete c;
}

TEST_F(MultipartCallStoreTest, free) {
  // The result of this test is only applicable in cooperation with valgrind

  supla_multipart_call_store store;
  store.add(new supla_multipart_call(1, 1000));
  store.add(new supla_multipart_call(2, 1000));
  store.add(new supla_multipart_call(3, 1000));
}

}  // namespace testing
