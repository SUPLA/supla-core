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

#include "integration/vbt/ValueBasedTriggerIntegrationTest.h"

#include <string>

#include "vbt/value_based_triggers.h"

using std::string;

namespace testing {

ValueBasedTriggerIntegrationTest::ValueBasedTriggerIntegrationTest()
    : IntegrationTest(), Test() {}

ValueBasedTriggerIntegrationTest::~ValueBasedTriggerIntegrationTest() {}

void ValueBasedTriggerIntegrationTest::SetUp() {
  initTestDatabase();
  runSqlScript("AddValueBasedTriggers.sql");
  Test::SetUp();
}

TEST_F(ValueBasedTriggerIntegrationTest, loadFromUserWhoHasNoTriggers) {
  supla_user *user = supla_user::find(12345, true);

  supla_value_based_triggers triggers(user);
  triggers.load();

  EXPECT_EQ(triggers.count(), 0);

  delete user;
}

TEST_F(ValueBasedTriggerIntegrationTest, loadAndDisableTwo) {
  supla_user *user = supla_user::find(2, true);

  supla_value_based_triggers triggers(user);
  triggers.load();

  EXPECT_EQ(triggers.count(), 6);

  runSqlScript("DisableTwoValueBasedTriggers.sql");
  triggers.load();

  EXPECT_EQ(triggers.count(), 4);
  EXPECT_TRUE(triggers.get(20) != nullptr);
  EXPECT_TRUE(triggers.get(21) != nullptr);
  EXPECT_TRUE(triggers.get(30) != nullptr);
  EXPECT_TRUE(triggers.get(32) != nullptr);

  delete user;
}

} /* namespace testing */
