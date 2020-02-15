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

#include "CDContainerTest.h"
#include "CDBaseMock.h"
#include "STCDContainer.h"
#include "gtest/gtest.h"  // NOLINT

namespace {

class CDContainerTest : public ::testing::Test {
 protected:
  STCDContainer *c;
};

TEST_F(CDContainerTest, addToList) {
  STCDContainer *container = new STCDContainer();
  ASSERT_FALSE(container == NULL);

  CDBaseMock *cd = new CDBaseMock(NULL);
  ASSERT_FALSE(cd == NULL);

  ASSERT_EQ(0, container->count());
  container->addToList(cd);
  container->addToList(cd);
  ASSERT_EQ(1, container->count());

  ASSERT_EQ(0, container->delCount());
  container->deleteAll(0);
  ASSERT_EQ(1, container->delCount());

  container->deleteAll(0);
  delete container;
}

TEST_F(CDContainerTest, moveToTrashWithoutPtrUse) {
  STCDContainer *container = new STCDContainer();
  ASSERT_FALSE(container == NULL);

  CDBaseMock *cd = new CDBaseMock(NULL);
  ASSERT_FALSE(cd == NULL);

  ASSERT_EQ(0, container->trashCount());
  ASSERT_EQ(0, container->count());
  container->addToList(cd);
  ASSERT_EQ(1, container->count());
  ASSERT_EQ(0, container->trashCount());
  container->moveToTrash(cd);
  ASSERT_EQ(0, container->count());
  ASSERT_EQ(0, container->trashCount());
  ASSERT_EQ(1, container->delCount());

  container->deleteAll(0);
  delete container;
}

TEST_F(CDContainerTest, moveToTrashWithPtrUse) {
  STCDContainer *container = new STCDContainer();
  ASSERT_FALSE(container == NULL);

  CDBaseMock *cd = new CDBaseMock(NULL);
  ASSERT_FALSE(cd == NULL);

  container->addToList(cd);
  cd->retainPtr();
  container->moveToTrash(cd);
  ASSERT_EQ(0, container->count());
  ASSERT_EQ(1, container->trashCount());
  ASSERT_EQ(0, container->delCount());
  cd->releasePtr();
  container->deleteAll(0);
  ASSERT_EQ(0, container->count());
  ASSERT_EQ(0, container->trashCount());
  ASSERT_EQ(1, container->delCount());

  delete container;
}

TEST_F(CDContainerTest, findItem) {
  STCDContainer *container = new STCDContainer();
  ASSERT_FALSE(container == NULL);

  CDBaseMock *cd = new CDBaseMock(NULL);
  ASSERT_FALSE(cd == NULL);
  ASSERT_EQ(NULL, container->findByPtr(cd));

  container->addToList(cd);
  ASSERT_FALSE(cd->ptrIsUsed());
  ASSERT_EQ(NULL, container->findByPtr((void *)1));
  ASSERT_EQ(cd, container->findByPtr(cd));
  ASSERT_TRUE(cd->ptrIsUsed());
  container->releasePtr(cd);
  ASSERT_FALSE(cd->ptrIsUsed());
  container->moveToTrash(cd);
  ASSERT_EQ(NULL, container->findByPtr(cd));

  container->deleteAll(0);
  delete container;
}

TEST_F(CDContainerTest, getItem) {
  STCDContainer *container = new STCDContainer();
  ASSERT_FALSE(container == NULL);

  CDBaseMock *cd = new CDBaseMock(NULL);
  ASSERT_FALSE(cd == NULL);
  ASSERT_EQ(NULL, container->get(0));

  container->addToList(cd);
  ASSERT_FALSE(cd->ptrIsUsed());
  ASSERT_EQ(NULL, container->get(1));
  ASSERT_EQ(cd, container->get(0));
  ASSERT_TRUE(cd->ptrIsUsed());
  container->releasePtr(cd);
  ASSERT_FALSE(cd->ptrIsUsed());
  container->moveToTrash(cd);
  ASSERT_EQ(NULL, container->get(0));

  container->deleteAll(0);
  delete container;
}

}  // namespace
