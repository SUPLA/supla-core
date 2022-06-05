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

#include "DObjectsBasicTest.h"

#include "doubles/distributedobjects/DObjectMock.h"

namespace testing {

using ::testing::_;

DObjectBasicTest::DObjectBasicTest(void) {
  objects = NULL;
  remoteUpdater = NULL;
}
DObjectBasicTest::~DObjectBasicTest(void) {}

void DObjectBasicTest::SetUp() {
  Test::SetUp();

  remoteUpdater = new DObjectRemoteUpdaterMock(this, 10);
  ASSERT_TRUE(remoteUpdater != NULL);

  objects = new DObjectsMock(remoteUpdater);
  ASSERT_TRUE(objects != NULL);
}

void DObjectBasicTest::TearDown() {
  Test::TearDown();
  if (objects) {
    delete objects;
  }

  if (remoteUpdater) {
    delete remoteUpdater;
  }
}

TEST_F(DObjectBasicTest, objectsWithoutChangeIndicator) {
  DObjectMock *obj = new DObjectMock(10);
  objects->add(obj);

  obj = new DObjectMock(20);
  objects->add(obj);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin(_)).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end(_, _)).Times(0);
  EXPECT_CALL(*remoteUpdater, prepare_the_update(_, _, _, _)).Times(0);

  objects->update_remote();
}

} /* namespace testing */
