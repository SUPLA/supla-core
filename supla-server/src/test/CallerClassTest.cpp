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

#include "CallerClassTest.h"

#include "caller.h"

namespace testing {

CallerClassTest::CallerClassTest(void) {}
CallerClassTest::~CallerClassTest(void) {}

TEST_F(CallerClassTest, constructorsWithoutParent) {
  supla_caller *caller = new supla_caller();
  EXPECT_TRUE(caller != NULL);
  if (caller) {
    EXPECT_EQ(caller->stack_size(), 1);
    EXPECT_EQ(caller->get_id(), 0);
    EXPECT_EQ(caller->get_type(), ctUnknown);
    delete caller;
  }

  caller = new supla_caller(ctGoogleHome);
  EXPECT_TRUE(caller != NULL);
  if (caller) {
    EXPECT_EQ(caller->stack_size(), 1);
    EXPECT_EQ(caller->get_id(), 0);
    EXPECT_EQ(caller->get_type(), ctGoogleHome);
    delete caller;
  }

  caller = new supla_caller(ctDevice, 123);
  EXPECT_TRUE(caller != NULL);
  if (caller) {
    EXPECT_EQ(caller->stack_size(), 1);
    EXPECT_EQ(caller->get_id(), 123);
    EXPECT_EQ(caller->get_type(), ctDevice);
    delete caller;
  }
}

TEST_F(CallerClassTest, constructorsWithParent) {
  // The path ctClient->ctGoogleHome is not possible. This is just an example
  // for the parent test.

  supla_caller *caller =
      new supla_caller(supla_caller(ctClient, 123), ctGoogleHome);
  EXPECT_TRUE(caller != NULL);
  if (caller) {
    EXPECT_EQ(caller->stack_size(), 2);
    EXPECT_EQ(caller->get_id(), 123);
    EXPECT_EQ(caller->get_type(), ctClient);
    EXPECT_EQ(caller->find(ctClient, 1), 0);
    EXPECT_EQ(caller->find(ctClient, 123), 1);
    EXPECT_EQ(caller->find(ctGoogleHome), 2);
    delete caller;
  }

  caller = new supla_caller(supla_caller(ctIPC), ctScene, 123);
  EXPECT_TRUE(caller != NULL);
  if (caller) {
    EXPECT_EQ(caller->stack_size(), 2);
    EXPECT_EQ(caller->get_id(), 0);
    EXPECT_EQ(caller->get_type(), ctIPC);
    EXPECT_EQ(caller->find(ctScene, 123), 2);
    EXPECT_EQ(caller->find(ctIPC), 1);
    delete caller;
  }

  caller = new supla_caller(
      supla_caller(supla_caller(supla_caller(ctScene, 1), ctScene, 2), ctScene,
                   3),
      ctScene, 4);
  EXPECT_TRUE(caller != NULL);
  if (caller) {
    EXPECT_EQ(caller->stack_size(), 4);
    EXPECT_EQ(caller->get_id(), 1);
    EXPECT_EQ(caller->get_type(), ctScene);
    EXPECT_EQ(caller->find(ctScene, 1), 1);
    EXPECT_EQ(caller->find(ctScene, 2), 2);
    EXPECT_EQ(caller->find(ctScene, 3), 3);
    EXPECT_EQ(caller->find(ctScene, 4), 4);
    EXPECT_EQ(caller->find(ctScene, 5), 0);
    delete caller;
  }
}

TEST_F(CallerClassTest, assignmentOperator) {
  supla_caller *c1 = new supla_caller(
      supla_caller(supla_caller(supla_caller(ctScene, 1), ctScene, 2), ctScene,
                   3),
      ctScene, 4);
  EXPECT_TRUE(c1 != NULL);
  if (c1) {
    EXPECT_EQ(c1->find(ctScene, 1), 1);
    EXPECT_EQ(c1->find(ctScene, 2), 2);
    EXPECT_EQ(c1->find(ctScene, 3), 3);
    EXPECT_EQ(c1->find(ctScene, 4), 4);
  }

  supla_caller *c2 = new supla_caller(
      supla_caller(supla_caller(supla_caller(ctScene, 10), ctScene, 20),
                   ctScene, 30),
      ctScene, 40);
  EXPECT_TRUE(c2 != NULL);
  if (c2) {
    EXPECT_EQ(c2->find(ctScene, 10), 1);
    EXPECT_EQ(c2->find(ctScene, 20), 2);
    EXPECT_EQ(c2->find(ctScene, 30), 3);
    EXPECT_EQ(c2->find(ctScene, 40), 4);
  }

  if (c1 && c2) {
    *c1 = *c2;
    EXPECT_EQ(c1->find(ctScene, 10), 1);
    EXPECT_EQ(c1->find(ctScene, 20), 2);
    EXPECT_EQ(c1->find(ctScene, 30), 3);
    EXPECT_EQ(c1->find(ctScene, 40), 4);
  }

  if (c1) {
    delete c1;
    c1 = NULL;
  }

  if (c2) {
    delete c2;
    c2 = NULL;
  }

  supla_caller c3(ctScene, 5);
  supla_caller c4;
  EXPECT_EQ(c4.get_id(), 0);
  EXPECT_EQ(c4.get_type(), ctUnknown);

  c4 = c3;
  EXPECT_EQ(c3.get_id(), 5);
  EXPECT_EQ(c3.get_type(), ctScene);
  EXPECT_TRUE(c3.get_id() == c4.get_id());
  EXPECT_TRUE(c3.get_type() == c4.get_type());
}

TEST_F(CallerClassTest, conversionToSenderId) {
  supla_caller c1(ctUnknown, 10);
  supla_caller c2(ctDevice, 10);
  supla_caller c3(ctClient, 10);
  supla_caller c4(ctIPC, 10);
  supla_caller c5(ctAmazonAlexa, 10);
  supla_caller c6(ctGoogleHome, 10);
  supla_caller c7(ctActionTrigger, 10);
  supla_caller c8(ctScene, 10);

  EXPECT_EQ(c1.convert_to_sender_id(), 0);
  EXPECT_EQ(c2.convert_to_sender_id(), 0);
  EXPECT_EQ(c3.convert_to_sender_id(), 10);
  EXPECT_EQ(c4.convert_to_sender_id(), 0);
  EXPECT_EQ(c5.convert_to_sender_id(), 0);
  EXPECT_EQ(c6.convert_to_sender_id(), 0);
  EXPECT_EQ(c7.convert_to_sender_id(), 0);
  EXPECT_EQ(c8.convert_to_sender_id(), 0);
}

TEST_F(CallerClassTest, comparsionOperator) {
  supla_caller c1;
  supla_caller c2(ctDevice);

  EXPECT_TRUE(c1 == ctUnknown);
  EXPECT_FALSE(c1 == ctDevice);

  EXPECT_FALSE(c1 != ctUnknown);
  EXPECT_TRUE(c1 != ctDevice);
}

} /* namespace testing */
