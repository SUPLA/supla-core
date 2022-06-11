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

#include "distributedobjects/dobject_change_indicator.h"
#include "doubles/distributedobjects/DObjectMock.h"
#include "srpc_adapter.h"

namespace testing {

using ::testing::_;

DObjectBasicTest::DObjectBasicTest(void) {
  objects = NULL;
  remoteUpdater = NULL;
  srpcAdapter = NULL;
}
DObjectBasicTest::~DObjectBasicTest(void) {}

void DObjectBasicTest::SetUp() {
  Test::SetUp();

  srpcAdapter = new SrpcAdapterMock(NULL);

  remoteUpdater = new DObjectRemoteUpdaterMock(srpcAdapter);
  ASSERT_TRUE(remoteUpdater != NULL);

  objects = new DObjectsMock(remoteUpdater);
  ASSERT_TRUE(objects != NULL);

  ON_CALL(*remoteUpdater, is_protocol_version_allowed)
      .WillByDefault(Return(true));
  ON_CALL(*remoteUpdater, on_transaction_begin).WillByDefault(Return(true));
  ON_CALL(*remoteUpdater, prepare_the_update).WillByDefault(Return(true));
  ON_CALL(*srpcAdapter, get_proto_version).WillByDefault(Return(18));
}

void DObjectBasicTest::TearDown() {
  Test::TearDown();
  if (objects) {
    delete objects;
  }

  if (remoteUpdater) {
    delete remoteUpdater;
  }

  if (srpcAdapter) {
    delete srpcAdapter;
  }
}

TEST_F(DObjectBasicTest, objectsWithoutChangeIndicator) {
  DObjectMock *obj = new DObjectMock(10);
  objects->add(obj);

  obj = new DObjectMock(20);
  objects->add(obj);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(0);
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(0);

  EXPECT_FALSE(objects->update_remote());
}

TEST_F(DObjectBasicTest, objectsWithChangeIndicator_Unchanged) {
  DObjectMock *obj = new DObjectMock(10);
  obj->set_change_indicator(new supla_dobject_change_indicator(false));
  objects->add(obj);

  obj = new DObjectMock(20);
  obj->set_change_indicator(new supla_dobject_change_indicator(false));
  objects->add(obj);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(0);
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(0);

  EXPECT_FALSE(objects->update_remote());
}

TEST_F(DObjectBasicTest, objectsWithChangeIndicator_Changed) {
  DObjectMock *obj1 = new DObjectMock(10);
  obj1->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj1);

  DObjectMock *obj2 = new DObjectMock(20);
  obj2->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj2);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin(obj1, 18)).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update(obj1, _, _)).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update(obj2, _, _)).Times(1);
  EXPECT_CALL(*remoteUpdater, on_transaction_end(srpcAdapter)).Times(1);

  EXPECT_TRUE(objects->update_remote());
}

TEST_F(DObjectBasicTest, failedToPrepareData) {
  DObjectMock *obj1 = new DObjectMock(10);
  obj1->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj1);

  DObjectMock *obj2 = new DObjectMock(20);
  obj2->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj2);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin(obj1, 18)).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update(obj1, _, _))
      .Times(1)
      .WillOnce(Return(false));
  EXPECT_CALL(*remoteUpdater, prepare_the_update(obj2, _, _))
      .Times(1)
      .WillOnce(Return(false));
  EXPECT_CALL(*remoteUpdater, on_transaction_end(srpcAdapter)).Times(1);

  EXPECT_FALSE(objects->update_remote());
}

TEST_F(DObjectBasicTest, stopProcessing) {
  DObjectMock *obj1 = new DObjectMock(10);
  obj1->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj1);

  DObjectMock *obj2 = new DObjectMock(20);
  obj2->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj2);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin(obj1, 18)).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update(obj1, _, _))
      .Times(1)
      .WillOnce(DoAll(SetArgPointee<2>(true), Return(true)));
  EXPECT_CALL(*remoteUpdater, prepare_the_update(obj2, _, _)).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end(srpcAdapter)).Times(1);

  EXPECT_TRUE(objects->update_remote());
}

TEST_F(DObjectBasicTest, stopProcessingRightAtTheBegining) {
  DObjectMock *obj1 = new DObjectMock(10);
  obj1->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj1);

  DObjectMock *obj2 = new DObjectMock(20);
  obj2->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj2);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin)
      .Times(1)
      .WillOnce(Return(false));
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(0);

  EXPECT_FALSE(objects->update_remote());
}

TEST_F(DObjectBasicTest, notAllowedVersion) {
  DObjectMock *obj1 = new DObjectMock(10);
  obj1->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj1);

  DObjectMock *obj2 = new DObjectMock(20);
  obj2->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj2);

  EXPECT_EQ(objects->count(), 2);

  EXPECT_CALL(*remoteUpdater, is_protocol_version_allowed)
      .WillOnce(Return(false));
  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(0);
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(0);

  EXPECT_FALSE(objects->update_remote());
}

TEST_F(DObjectBasicTest, stillChanged) {
  // Check behavior without setting change indicators

  DObjectMock *obj1 = new DObjectMock(10);
  obj1->set_change_indicator(new supla_dobject_change_indicator(true));
  objects->add(obj1);

  EXPECT_EQ(objects->count(), 1);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(1);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(1);

  EXPECT_TRUE(objects->update_remote());

  EXPECT_TRUE(obj1->get_change_indicator() == NULL);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(0);
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(0);

  EXPECT_FALSE(objects->update_remote());

  // Essential test

  obj1->set_change_indicator(new supla_dobject_change_indicator(true));

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update)
      .Times(1)
      .WillOnce(
          DoAll(SetArgPointee<1>(new supla_dobject_change_indicator(false)),
                Return(true)));

  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(1);

  EXPECT_TRUE(objects->update_remote());

  EXPECT_TRUE(obj1->get_change_indicator() != NULL);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(0);
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(0);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(0);

  EXPECT_FALSE(objects->update_remote());

  obj1->set_change_indicator(new supla_dobject_change_indicator(true));

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update)
      .Times(1)
      .WillOnce(
          DoAll(SetArgPointee<1>(new supla_dobject_change_indicator(true)),
                Return(true)));  // Still changed

  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(1);

  EXPECT_TRUE(objects->update_remote());

  EXPECT_TRUE(obj1->get_change_indicator() != NULL);

  EXPECT_CALL(*remoteUpdater, on_transaction_begin).Times(1);
  EXPECT_CALL(*remoteUpdater, prepare_the_update).Times(1);
  EXPECT_CALL(*remoteUpdater, on_transaction_end).Times(1);

  EXPECT_TRUE(objects->update_remote());  // On the next attempt, the object was
                                          // marked as unchanged

  EXPECT_TRUE(obj1->get_change_indicator() == NULL);
}

} /* namespace testing */
