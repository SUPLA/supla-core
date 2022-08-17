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

#include "ConnectionObjectsTest.h"

#include <memory>

#include "doubles/conn/ConnectionObjectMock.h"
#include "doubles/conn/ConnectionObjectsMock.h"
#include "gtest/gtest.h"  // NOLINT

namespace testing {

using std::make_shared;
using std::shared_ptr;
using std::vector;

TEST_F(ConnectionObjectsTest, forEach) {
  ConnectionObjectsMock objects;

  shared_ptr<ConnectionObjectMock> cd1 =
      make_shared<ConnectionObjectMock>(nullptr);

  shared_ptr<ConnectionObjectMock> cd2 =
      make_shared<ConnectionObjectMock>(nullptr);

  shared_ptr<ConnectionObjectMock> cd3 =
      make_shared<ConnectionObjectMock>(nullptr);

  objects.add(cd1);
  objects.add(cd2);
  objects.add(cd3);
  objects.add(cd3);

  EXPECT_EQ(3, objects.count());
  cd2 = nullptr;
  EXPECT_EQ(2, objects.count());

  short counter = 0;
  objects.for_each([&counter, cd1, cd2,
                    cd3](std::shared_ptr<supla_connection_object> obj) -> bool {
    if (obj == cd1) {
      counter++;
    }
    if (obj == cd2) {
      counter++;
    }
    if (obj == cd3) {
      counter++;
    }
    return true;
  });

  EXPECT_EQ(2, counter);

  counter = 0;
  objects.for_each([&counter, cd1, cd2,
                    cd3](std::shared_ptr<supla_connection_object> obj) -> bool {
    counter++;
    return true;
  });

  EXPECT_EQ(2, counter);

  counter = 0;
  objects.for_each([&counter, cd1, cd2,
                    cd3](std::shared_ptr<supla_connection_object> obj) -> bool {
    counter++;
    return false;
  });

  EXPECT_EQ(1, counter);
}

TEST_F(ConnectionObjectsTest, getAll) {
  ConnectionObjectsMock objects;

  shared_ptr<ConnectionObjectMock> cd1 =
      make_shared<ConnectionObjectMock>(nullptr);

  shared_ptr<ConnectionObjectMock> cd2 =
      make_shared<ConnectionObjectMock>(nullptr);

  shared_ptr<ConnectionObjectMock> cd3 =
      make_shared<ConnectionObjectMock>(nullptr);

  objects.add(cd1);
  objects.add(cd2);
  objects.add(cd3);
  cd2 = nullptr;

  short counter = 0;

  vector<shared_ptr<supla_connection_object> > _objects = objects.get_all();

  for (auto it = _objects.begin(); it != _objects.end(); ++it) {
    if (*it == cd1) {
      counter++;
    }
    if (*it == cd2) {
      counter++;
    }
    if (*it == cd3) {
      counter++;
    }
  }

  EXPECT_EQ(2, counter);
  EXPECT_EQ(2, _objects.size());
}

TEST_F(ConnectionObjectsTest, add) {
  ConnectionObjectsMock objects;
  EXPECT_EQ(0, objects.count());

  shared_ptr<ConnectionObjectMock> cd1 =
      make_shared<ConnectionObjectMock>(nullptr);

  shared_ptr<ConnectionObjectMock> cd2 =
      make_shared<ConnectionObjectMock>(nullptr);

  objects.add(cd1);
  EXPECT_EQ(1, objects.count());
  objects.add(cd2);
  EXPECT_EQ(2, objects.count());
  objects.add(cd1);
  EXPECT_EQ(2, objects.count());

  cd1 = nullptr;
  EXPECT_EQ(1, objects.count());
}

TEST_F(ConnectionObjectsTest, get) {
  ConnectionObjectsMock objects;

  shared_ptr<ConnectionObjectMock> cd1 =
      make_shared<ConnectionObjectMock>(nullptr);
  cd1->set_id(11);

  shared_ptr<ConnectionObjectMock> cd2 =
      make_shared<ConnectionObjectMock>(nullptr);
  cd2->set_id(12);

  objects.add(cd1);
  objects.add(cd2);

  EXPECT_TRUE(objects.get(10) == nullptr);
  EXPECT_TRUE(objects.get(11) == cd1);
  EXPECT_TRUE(objects.get(12) == cd2);
  cd1 = nullptr;
  EXPECT_TRUE(objects.get(11) == nullptr);
  EXPECT_TRUE(objects.get(55) == nullptr);
  cd2->set_id(55);
  EXPECT_TRUE(objects.get(55) == cd2);
}

}  // namespace testing
